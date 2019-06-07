type id;

type data;

type position = {
  scrollPosition: int,
  heightMap: Belt.HashMap.Int.t(int),
};

type rectangle = {
  top: int,
  height: int,
};

type snapShot = {
  rectangles: Belt.HashMap.Int.t(rectangle),
  startIndex: int,
  endIndex: int,
  scrollTop: int,
  heightMap: Belt.HashMap.Int.t(int),
};

type state = {
  startIndex: int,
  endIndex: int,
};

[@bs.val] external setTimeout: (unit => unit, int) => id = "setTimeout";
[@bs.val] external clearTimeout: id => unit = "clearTimeout";

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

let sortByKey = (a, b) => {
  let (id_a, _item_a) = a;

  let (id_b, _item_b) = b;

  switch (id_a > id_b) {
  | true => 1
  | false when id_a === id_b => 0
  | _ => (-1)
  };
};

let defaultPositionValue = {
  scrollPosition: 15000,
  heightMap: Belt.HashMap.Int.make(~hintSize=100),
};

let isBetween = (target, beginning, endValue) => {
  target >= beginning && target <= endValue;
};

let doesIntersectWith = (a, b) =>
  isBetween(a.top, b.top, b.top + b.height)
  || isBetween(b.top, a.top, a.top + a.height);

let createNewRecs = (~heightMap, ~identity, ~defaultHeight, ~data) => {
  let recs = Belt.HashMap.Int.make(~hintSize=100);

  let currentHeight = ref(0);

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

  heightMap
  ->convertToSortedArray
  ->Belt.Array.forEach(item => {
      let (id, height) = item;

      Belt.HashMap.Int.set(recs, id, {top: currentHeight^, height});

      currentHeight := currentHeight^ + height;
    });

  recs;
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
  let ({startIndex, endIndex}: state, setIndex) =
    React.useState(() => {startIndex: (-1), endIndex: 10});

  let (_corr, setCorrection) = React.useState(() => 0);

  let refMap = React.useRef(Belt.HashMap.Int.make(~hintSize=100));

  let heightMap = React.useRef(defaultPosition.heightMap);

  let recMap = React.useRef(Belt.HashMap.Int.make(~hintSize=100));

  let scrollTopPosition = React.useRef(0);

  let viewPortRec = React.useRef({top: 0, height: 0});

  let prevViewPortRec = React.useRef({top: 0, height: 0});

  let previousSnapshot =
    React.useRef({
      startIndex: 0,
      endIndex: 0,
      scrollTop: 0,
      rectangles: Belt.HashMap.Int.make(~hintSize=100),
      heightMap: Belt.HashMap.Int.make(~hintSize=100),
    });

  let element =
    viewPortRef
    ->React.Ref.current
    ->Js.Nullable.toOption
    ->Belt.Option.map(Webapi.Dom.Element.unsafeAsHtmlElement);

  let handleScroll = _e => {
    switch (element) {
    | Some(element) =>
      let startItem =
        data
        ->Belt.Array.map(rawData =>
            recMap
            ->React.Ref.current
            ->Belt.HashMap.Int.get(rawData->identity)
            ->Belt.Option.mapWithDefault(
                (rawData->identity, {top: 0, height: 0}), rectangle =>
                (rawData->identity, rectangle)
              )
          )
        ->Belt.Array.reduce(
            (0, {top: 0, height: 0}),
            (sum, item) => {
              let (_sumId, sumRect) = sum;

              let yy = viewPortRec->React.Ref.current;

              sumRect.top + sumRect.height > yy.top ? sum : item;
            },
          );

      let endItem =
        data
        ->Belt.Array.map(i =>
            switch (
              recMap->React.Ref.current->Belt.HashMap.Int.get(i->identity)
            ) {
            | Some(h) => (i->identity, h)
            | None => (i->identity, {top: 0, height: 0})
            }
          )
        ->Belt.Array.reduce(
            (0, {top: 0, height: 0}),
            (sum, item) => {
              let (_sumId, sumRect) = sum;
              // find the first item that is in the viewport
              // and thean calcualte correction
              let yy = viewPortRec->React.Ref.current;

              yy.top + yy.height <= sumRect.top ? sum : item;
              // switch (rect->y, sumRect->y) {
              // | (true, false) => item
              // | _ => sum
              // };
            },
          );

      setIndex(_prev => {
        React.Ref.setCurrent(
          previousSnapshot,
          {
            ...React.Ref.current(previousSnapshot),
            startIndex: _prev.startIndex,
          },
        );

        React.Ref.setCurrent(
          previousSnapshot,
          {
            ...React.Ref.current(previousSnapshot),
            endIndex: _prev.endIndex,
            scrollTop: element->scrollTop->int_of_float,
            heightMap: heightMap->React.Ref.current->Belt.HashMap.Int.copy,
          },
        );

        let viewPortReci: rectangle = {
          height: Webapi.Dom.HtmlElement.clientHeight(element),
          top: Webapi.Dom.HtmlElement.scrollTop(element)->int_of_float,
        };

        prevViewPortRec->React.Ref.setCurrent(viewPortRec->React.Ref.current);

        viewPortRec->React.Ref.setCurrent(viewPortReci);

        let (sid, _item) = startItem;
        let (eid, _item) = endItem;

        switch (
          _prev.startIndex == (sid - bufferCount < 0 ? 0 : sid - bufferCount),
          _prev.endIndex
          == (
               eid + bufferCount > data->Belt.Array.length
                 ? data->Belt.Array.length : eid + bufferCount
             ),
        ) {
        | (true, true) => _prev
        | _ => {
            startIndex: sid - bufferCount < 1 ? 1 : sid - bufferCount,
            endIndex:
              eid + bufferCount > data->Belt.Array.length
                ? data->Belt.Array.length : eid + bufferCount,
          }
        };
      });

    | _ => ()
    };

    setCorrection(x => x + 1)->ignore;
  };

  React.useEffect1(
    () => {
      setTimeout(
        () => setIndex(prev => {startIndex: 0, endIndex: prev.endIndex}),
        1,
      )
      ->ignore;

      None;
    },
    [||],
  );

  /**
   *
   */
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
        1000,
      )
      ->ignore;

      None;
    },
    [||],
  );

  /**
   * Attaches the eventlistener to the viewport.
   */
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

      let prevRec = Belt.HashMap.Int.copy(recMap->current);

      refMap
      ->current
      ->Belt.HashMap.Int.forEach((key, item) =>
          calculateHeight(item, heightMap, key)
        );

      let recs = createNewRecs(~heightMap, ~identity, ~defaultHeight, ~data);

      recMap->setCurrent(recs);

      let findAnchor = () => {
        let prev = previousSnapshot->current;

        let both =
          data->Belt.Array.keep(item => {
            let id = item->identity;

            prev.startIndex <= id
            && prev.endIndex >= id
            && startIndex <= id
            && endIndex >= id
              ? true : false;
          });

        let interSectWithPrevViewPortPosition =
          prevViewPortRec->current->doesIntersectWith;

        let anchor: (int, rectangle) =
          both
          ->map(item =>
              prevRec
              ->Belt.HashMap.Int.get(item->identity)
              ->Belt.Option.mapWithDefault(
                  (item->identity, {top: 0, height: 0}), rectangle =>
                  (item->identity, rectangle)
                )
            )
          ->reduce(
              (0, {top: 0, height: 0}),
              (best, current) => {
                let (_id, currentRectangle) = current;
                let (_sumId, bestRectangle) = best;

                currentRectangle->interSectWithPrevViewPortPosition
                && !bestRectangle->interSectWithPrevViewPortPosition
                  ? current : best;
              },
            );

        let (anchorId, prevAnchorRectangle) = anchor;

        let currentAnchorRectangle =
          recs
          ->Belt.HashMap.Int.get(anchorId)
          ->Belt.Option.mapWithDefault({top: 0, height: 0}, x => x);

        let correction = currentAnchorRectangle.top - prevAnchorRectangle.top;

        switch (element, correction > 0) {
        | (Some(viewport), true) =>
          Webapi.Dom.HtmlElement.scrollByWithOptions(
            {
              "top": correction->float_of_int,
              "left": 0.0,
              "behavior": "auto",
            },
            viewport,
          )
        | (_, _) => ()
        };
      };

      heightDelta(
        ~data,
        ~identity,
        ~rectangles=recs,
        ~previousRectangles=prevRec,
      )
      === 0
        ? () : findAnchor();

      None;
    },
    [|startIndex, endIndex|],
  );

  let startPadding =
    recMap
    ->React.Ref.current
    ->Belt.HashMap.Int.get(startIndex)
    ->Belt.Option.mapWithDefault(0, x => x.top);

  let endPadding = {
    let endValue =
      recMap
      ->React.Ref.current
      ->Belt.HashMap.Int.get(endIndex)
      ->Belt.Option.mapWithDefault(0, x => x.top);

    let lastValue =
      recMap
      ->React.Ref.current
      ->Belt.HashMap.Int.get(
          Belt.HashMap.Int.size(recMap->React.Ref.current),
        )
      ->Belt.Option.mapWithDefault(0, x => x.top);

    lastValue - endValue;
  };

  <List
    data={
      data->Belt.Array.keep(item =>
        item->identity <= endIndex && item->identity >= startIndex
      )
    }
    identity
    renderItem
    beforePadding=startPadding
    afterPadding=endPadding
    onRefChange={(id, elementRef) =>
      Belt.HashMap.Int.set(refMap->React.Ref.current, id, elementRef)
    }
  />;
};