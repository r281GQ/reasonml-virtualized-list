type keyProps = {. ref: string};

type id;

type data;

type rectangle = {
  top: int,
  height: int,
};

[@bs.val] external setTimeout: (unit => unit, int) => id = "setTimeout";
[@bs.val] external clearTimeout: id => unit = "clearTimeout";

let foldOnHeight = (sum: int, item: (int, int)) => {
  let (_id, height) = item;

  sum + height;
};

let recsHeight =
    (
      data: array('data),
      identity: 'data => int,
      rectangles: Belt.HashMap.Int.t(rectangle),
    )
    : int =>
  Belt.(
    data
    ->Array.get(data->Array.length - 1)
    ->Option.map(identity)
    ->Option.flatMap(id => rectangles->HashMap.Int.get(id))
    ->Option.mapWithDefault(0, item => item.height + item.top)
  );

let heightDelta = (~data, ~identity, ~rectangles, ~previousRectangles) =>
  recsHeight(data, identity, previousRectangles)
  - recsHeight(data, identity, rectangles);

let calculateHeight = (elementRef, heightMap, id) => {
  elementRef
  ->Js.Nullable.toOption
  ->Belt.Option.map(Webapi.Dom.HtmlElement.clientHeight)
  ->Belt.Option.map(height =>
      Belt.HashMap.Int.set(React.Ref.current(heightMap), id, height)
    )
  ->ignore;
};

let scrollTop = Webapi.Dom.HtmlElement.scrollTop;

let log = Js.log;

type position = {
  scrollPosition: int,
  heightMap: Belt.HashMap.Int.t(int),
};

type snapShot = {
  rectangles: Belt.HashMap.Int.t(rectangle),
  startIndex: int,
  endIndex: int,
  scrollTop: int,
  heightMap: Belt.HashMap.Int.t(int),
};

let defaultPositionValue = {
  scrollPosition: 15000,
  heightMap: Belt.HashMap.Int.make(100),
};

[@react.component]
let make =
    (
      ~bufferCount: int=5,
      ~defaultPosition: position=defaultPositionValue,
      ~onDestroy:
         (~scrollPosition: int, ~heightMap: Belt.HashMap.Int.t(int)) => unit=?,
      ~defaultHeight=200,
      ~data: array('data),
      ~identity: 'data => int,
      ~viewPortRef,
      ~renderItem: 'data => React.element,
    ) => {
  let (startIndex, setStartIndex) = React.useState(() => (-1));

  let (endIndex, setEndIndex) = React.useState(() => 10);

  let refMap = React.useRef(Belt.HashMap.Int.make(100));

  let heightMap = React.useRef(defaultPosition.heightMap);

  let recMap = React.useRef(Belt.HashMap.Int.make(100));

  let scrollTopPosition = React.useRef(0);
  let viewPortRec = React.useRef({top: 0, height: 0});
  let prevViewPortRec = React.useRef({top: 0, height: 0});

  let previousSnapshot =
    React.useRef({
      startIndex: 0,
      endIndex: 0,
      scrollTop: 0,
      rectangles: Belt.HashMap.Int.make(100),
      heightMap: Belt.HashMap.Int.make(100),
    });

  //previousSnapshot->React.Ref.current->;

  let sortByKey = (a, b) => {
    let (id_a, _item_a) = a;

    let (id_b, _item_b) = b;

    switch (id_a > id_b) {
    | true => 1
    | false when id_a === id_b => 0
    | _ => (-1)
    };
  };

  let convertToSortedArray = heightMap => {
    let map = React.Ref.current(heightMap);

    data
    ->Belt.Array.map(item => {
        let id = item->identity;

        (id, defaultHeight);
      })
    ->Belt.Array.map(item => {
        let (id, _height) = item;

        map
        ->Belt.HashMap.Int.get(id)
        ->Belt.Option.mapWithDefault(item, measuredHeight =>
            (id, measuredHeight)
          );
      })
    ->Belt.SortArray.stableSortBy(sortByKey);
  };

  let element =
    viewPortRef
    ->React.Ref.current
    ->Js.Nullable.toOption
    ->Belt.Option.map(Webapi.Dom.Element.unsafeAsHtmlElement);

  let handleScroll = _e => {
    switch (element) {
    | Some(element) =>
      // persist the current position as the prev

      setStartIndex(_prev => {
        React.Ref.setCurrent(
          previousSnapshot,
          {...React.Ref.current(previousSnapshot), startIndex: _prev},
        );

        let viewPortReci: rectangle = {
          height: Webapi.Dom.HtmlElement.clientHeight(element),
          top: Webapi.Dom.HtmlElement.scrollTop(element)->int_of_float,
        };

        prevViewPortRec->React.Ref.setCurrent(viewPortRec->React.Ref.current);

        viewPortRec->React.Ref.setCurrent(viewPortReci);

        let startItem =
          convertToSortedArray(heightMap)
          ->Belt.Array.reduce(
              (0, 0),
              (sum, item) => {
                let (id, height) = item;
                let (_prevId, sumHeight) = sum;

                sumHeight > int_of_float(scrollTop(element))
                  ? sum : (id, height + sumHeight);
              },
            );

        let (id, _item) = startItem;

        id - bufferCount < 0 ? 0 : id - bufferCount;
      });

      scrollTopPosition->React.Ref.setCurrent(
        element->scrollTop->int_of_float,
      );

      setEndIndex(_prev => {
        React.Ref.setCurrent(
          previousSnapshot,
          {
            ...React.Ref.current(previousSnapshot),
            endIndex: _prev,
            scrollTop: element->scrollTop->int_of_float,
            heightMap: heightMap->React.Ref.current->Belt.HashMap.Int.copy,
          },
        );
        let endIndex =
          heightMap
          ->convertToSortedArray
          ->Belt.Array.reduce(
              (0, 0),
              (sum, item) => {
                let (id, height) = item;
                let (_prevId, sumHeight) = sum;

                sumHeight > element->scrollTop->int_of_float
                + element->Webapi.Dom.HtmlElement.clientHeight
                  ? sum : (id, height + sumHeight);
              },
            );

        let (id, _item) = endIndex;

        id + bufferCount > data->Belt.Array.length
          ? data->Belt.Array.length : id + bufferCount;
      });

    | None => ()
    };
  };

  React.useEffect1(
    () => {
      setTimeout(() => setStartIndex(_ => 0), 1)->ignore;

      None;
    },
    [||],
  );

  React.useEffect1(
    () => {
      setTimeout(
        () => {
          let setScrollTop =
            viewPortRef
            ->React.Ref.current
            ->Js.Nullable.toOption
            ->Belt.Option.map(Webapi.Dom.Element.unsafeAsHtmlElement)
            ->Belt.Option.map(Webapi.Dom.HtmlElement.setScrollTop);

          switch (setScrollTop) {
          | Some(fn) => defaultPositionValue.scrollPosition->float_of_int->fn
          | None => ()
          };
        },
        100,
      )
      ->ignore;

      None;
    },
    [||],
  );

  React.useEffect1(
    () => {
      switch (element) {
      | Some(element) =>
        Webapi.Dom.HtmlElement.addEventListener(
          "scroll",
          handleScroll,
          element,
        )
      | None => ()
      };

      Some(
        () =>
          switch (element) {
          | Some(element) =>
            Webapi.Dom.HtmlElement.removeEventListener(
              "scroll",
              handleScroll,
              element,
            )
          | None => ()
          },
      );
    },
    [|element|],
  );

  React.useEffect1(
    () =>
      Some(
        () =>
          onDestroy(
            ~scrollPosition=scrollTopPosition->React.Ref.current,
            ~heightMap=heightMap->React.Ref.current,
          ),
      ),
    [||],
  );

  let startPadding =
    heightMap
    ->convertToSortedArray
    ->Belt.Array.slice(~len=startIndex, ~offset=0)
    ->Belt.Array.reduce(
        0,
        (sum, item) => {
          let (_id, height) = item;

          sum + height;
        },
      );

  let endPadding =
    heightMap
    ->convertToSortedArray
    ->Belt.Array.slice(
        ~len=Belt.Array.length(data) - endIndex,
        ~offset=endIndex,
      )
    ->Belt.Array.reduce(
        0,
        (sum, item) => {
          let (_id, height) = item;

          sum + height;
        },
      );

  /**
    This triggered by changing the indexes!

    Things to do!

    - save current rectangels to prev

    - grab new heights

    - calculate new rectangles

    - calculate height diff if any

    - if there is height diff find anchor and move scroller
   */
  React.useEffect1(
    () => {
      open React.Ref;
      open Belt.Array;

      startIndex->log;
      endIndex->log;

      React.Ref.setCurrent(
        previousSnapshot,
        {
          ...React.Ref.current(previousSnapshot),
          heightMap: heightMap->React.Ref.current->Belt.HashMap.Int.copy,
          rectangles: recMap->current,
        },
      );

      let currentHeight = heightMap->current;
      // let {heightMap as prevHeights} = previousSnapshot->current;

      let currentVP =
        heightMap->convertToSortedArray->reduce(0, foldOnHeight);

      let recs = Belt.HashMap.Int.make(100);

      let foo = ref(0);

      refMap
      ->current
      ->Belt.HashMap.Int.forEach((key, item) =>
          calculateHeight(item, heightMap, key)
        );

      heightMap
      ->convertToSortedArray
      ->forEach(item => {
          let (id, height) = item;

          Belt.HashMap.Int.set(recs, id, {top: foo^, height});

          foo := foo^ + height;
        });

      recMap->setCurrent(recs);

      let isBetween = (target, beginning, endValue) => {
        target >= beginning && target <= endValue;
      };

      let doesIntersectWith = (a, b) => {
        let d = isBetween(a.top, b.top, b.top + b.height);
        let ds = isBetween(b.top, a.top, a.top + a.height);

        d || ds;
      };

      let findAnchor = () => {
        prevViewPortRec->current->log;

        let viewPortReci: rectangle = {
          height:
            viewPortRef
            ->current
            ->Js.Nullable.toOption
            ->Belt.Option.map(Webapi.Dom.Element.unsafeAsHtmlElement)
            ->Belt.Option.mapWithDefault(
                0,
                Webapi.Dom.HtmlElement.clientHeight,
              ),
          top:
            viewPortRef
            ->current
            ->Js.Nullable.toOption
            ->Belt.Option.map(Webapi.Dom.Element.unsafeAsHtmlElement)
            ->Belt.Option.map(Webapi.Dom.HtmlElement.scrollTop)
            ->Belt.Option.mapWithDefault(0, int_of_float),
        };

        viewPortRec->current->log;
        viewPortReci->log;

        let prev = previousSnapshot->current;

        let both =
          data
          ->Belt.List.fromArray
          ->Belt.List.filter(item => {
              let id = item->identity;

              switch (
                prev.startIndex <= id && prev.endIndex >= id,
                startIndex <= id && endIndex >= id,
              ) {
              | (true, true) => true
              | _ => false
              };
            })
          ->Belt.List.toArray;

        let y = doesIntersectWith(prevViewPortRec->current);

        both->log;

        both
        ->map(i =>
            switch (recs->Belt.HashMap.Int.get(i->identity)) {
            | Some(h) => (i->identity, h)
            | None => (i->identity, {top: 0, height: 0})
            }
          )
        ->reduce(
            0,
            (sum, item) => {
              let (id, rect) = item;
              // find the first item that is in the viewport
              // and thean calcualte correction
              rect->y->log;

              0;
            },
          );
        ();
      };

      switch (
        heightDelta(
          ~data,
          ~identity,
          ~rectangles=recMap->current,
          ~previousRectangles=previousSnapshot->current.rectangles,
        )
      ) {
      | 0 => ()
      | _ => findAnchor()
      };

      let prevVP =
        data
        ->Belt.Array.map(item => {
            let id = item->identity;

            (id, defaultHeight);
          })
        ->Belt.Array.map(item => {
            let (id, _height) = item;

            let u = previousSnapshot->current;
            u.heightMap
            ->Belt.HashMap.Int.get(id)
            ->Belt.Option.mapWithDefault(item, measuredHeight =>
                (id, measuredHeight)
              );
          })
        ->Belt.SortArray.stableSortBy(sortByKey)
        ->reduce(0, foldOnHeight);

      None;
    },
    [|startIndex, endIndex|],
  );

  <React.Fragment>
    <div>
      <div
        className=Css.(
          style([
            paddingTop(px(startPadding)),
            paddingBottom(px(endPadding)),
          ])
        )>
        Belt.Array.(
          data
          ->slice(~offset=startIndex, ~len=endIndex - startIndex)
          ->map(item => (renderItem(item), identity(item)))
          ->map(itemTuple => {
              let (element, id) = itemTuple;

              ReasonReact.cloneElement(
                element,
                ~props={
                  "ref": elementRef =>
                    {Belt.HashMap.Int.set(
                       React.Ref.current(refMap),
                       id,
                       elementRef,
                     )},
                },
                [||],
              );
            })
        )
        ->ReasonReact.array
      </div>
    </div>
  </React.Fragment>;
};