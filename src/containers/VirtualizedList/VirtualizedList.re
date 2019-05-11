type keyProps = {. ref: string};

type id;

[@bs.val] external setTimeout: (unit => unit, int) => id = "setTimeout";

type data;

type rectangle = {
  top: int,
  bottom: int,
};

let scrollTop = Webapi.Dom.HtmlElement.scrollTop;

let log = Js.log;

type position = {
  scrollPosition: int,
  heightMap: Belt.HashMap.Int.t(int),
};

let defaultPositionValue = {
  scrollPosition: 1000,
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

  let scrollTopPosition = React.useRef(0);

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

  React.useEffect1(
    () => {
      setTimeout(
        () => {
          setStartIndex(_ => 0);

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
        1,
      )
      ->ignore;

      None;
    },
    [||],
  );

  let handleScroll = _e => {
    switch (element) {
    | Some(element) =>
      setStartIndex(_prev => {
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
                     )

                     elementRef
                     ->Js.Nullable.toOption
                     ->Belt.Option.map(Webapi.Dom.HtmlElement.clientHeight)
                     ->Belt.Option.map(height =>
                         Belt.HashMap.Int.set(
                           React.Ref.current(heightMap),
                           id,
                           height,
                         )
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