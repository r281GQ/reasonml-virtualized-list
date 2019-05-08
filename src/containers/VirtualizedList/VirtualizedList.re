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
  let (endIndex, _setEndindex) = React.useState(() => 10);

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
  let convertToSortedArray = heightMap =>
    React.Ref.current(heightMap)
    ->Belt.HashMap.Int.toArray
    ->Belt.SortArray.stableSortBy(sortByKey);

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
              _e =>
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
                  id;
                }),
              // setStartIndex(_prev => {
              //   let value = int_of_float(scrollTop(element) /. 200.);
              //   value - bufferCount < 0 ? 0 : value - bufferCount;
              // });
              // setEndindex(_prev => {
              //   let value =
              //     int_of_float(
              //       (
              //         scrollTop(element)
              //         +. float_of_int(
              //              Webapi.Dom.HtmlElement.clientHeight(element),
              //            )
              //       )
              //       /. 200.,
              //     );
              //   value + bufferCount >= 25 ? 25 + 1 : value + bufferCount;
              // });
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

  <React.Fragment>
    <div>
      <div
        className=Css.(
          style([
            paddingTop(px(startIndex * 200)),
            paddingBottom(px((Belt.Array.length(data) - endIndex) * 200)),
          ])
        )>
        Belt.Array.(
          data
          ->slice(startIndex, endIndex - startIndex)
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
  // </button>
  //   {ReasonReact.string("Trigger rerender")}
  //   onClick={_e => setStartIndex(prevIndex => prevIndex + 1)}>
  //   )
  //     style([display(block), marginLeft(auto), marginRight(auto)])
  //   className=Css.(
  // <button
};