type keyProps = {. ref: string};

type data;

type rectangle = {
  top: int,
  bottom: int,
};

let scrollTop = Webapi.Dom.HtmlElement.scrollTop;

[@react.component]
let make =
    (
      ~bufferCount=5,
      ~defaultHeight=200,
      ~data: array('data),
      ~identity: 'data => int,
      ~viewPortRef,
      ~renderItem: 'data => React.element,
    ) => {
  let (startIndex, setStartIndex) = React.useState(() => 0);
  let (endIndex, setEndIndex) = React.useState(() => 10);

  let refMap = React.useRef(Belt.HashMap.Int.make(100));

  let heightMap = React.useRef(Belt.HashMap.Int.make(100));

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

  React.useEffect1(
    () => {
      Belt.Option.(
        viewPortRef
        ->React.Ref.current
        ->Js.Nullable.toOption
        ->map(Webapi.Dom.Element.unsafeAsHtmlElement)
        ->map(element =>
            Webapi.Dom.HtmlElement.addEventListener(
              "scroll",
              _e => {
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
              },
              element,
            )
          )
      );

      Some(
        () => {
          Belt.Option.(
            viewPortRef
            ->React.Ref.current
            ->Js.Nullable.toOption
            ->map(Webapi.Dom.Element.unsafeAsHtmlElement)
            ->map(element =>
                Webapi.Dom.HtmlElement.removeEventListener(
                  "scroll",
                  _e => (),
                  element,
                )
              )
          );

          ();
        },
      );
    },
    [||],
  );

  React.useEffect1(() => None, [||]);

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