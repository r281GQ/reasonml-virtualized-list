type keyProps = {. ref: string};

let scrollTop = Webapi.Dom.HtmlElement.scrollTop;

type rectangle = {
  top: int,
  bottom: int,
};

[@react.component]
let make = (~data, ~renderItem, ~identity, ~innerRef) => {
  let viewPortRef = React.useRef(Js.Nullable.null);

  let (startIndex, setStartIndex) = React.useState(() => 0);
  let (endIndex, setEndindex) = React.useState(() => 10);

  let refMap = React.useRef(Belt.HashMap.Int.make(100));

  // let u =
  //   switch (viewPortRef->React.Ref.current->Js.Nullable.toOption) {
  //   | Some(refObject) => Belt.Option.map(Webapi.Dom.HtmlElement.clientHeight)
  //   | None => 333
  //   };

  let clientHeight =
    Belt.Option.(
      innerRef
      ->React.Ref.current
      ->Js.Nullable.toOption
      ->map(Webapi.Dom.Element.unsafeAsHtmlElement)
      ->map(Webapi.Dom.HtmlElement.clientHeight)
    );

  let srollHeight =
    Belt.Option.(
      viewPortRef
      ->React.Ref.current
      ->Js.Nullable.toOption
      ->map(Webapi.Dom.Element.unsafeAsHtmlElement)
      ->map(Webapi.Dom.HtmlElement.clientHeight)
    );

  React.useEffect1(
    () => {
      Belt.Option.(
        innerRef
        ->React.Ref.current
        ->Js.Nullable.toOption
        ->map(Webapi.Dom.Element.unsafeAsHtmlElement)
        ->map(element =>
            Webapi.Dom.HtmlElement.addEventListener(
              "scroll",
              _e => {
                setStartIndex(_prev =>
                  int_of_float(scrollTop(element) /. 200.)
                );

                setEndindex(_prev =>
                  int_of_float(
                    (
                      scrollTop(element)
                      +. float_of_int(
                           Webapi.Dom.HtmlElement.clientHeight(element),
                         )
                    )
                    /. 200.,
                  )
                );
              },
              element,
            )
          )
      );

      Some(
        () => {
          Belt.Option.(
            innerRef
            ->React.Ref.current
            ->Js.Nullable.toOption
            ->map(Webapi.Dom.Element.unsafeAsHtmlElement)
            ->map(element =>
                Webapi.Dom.HtmlElement.removeEventListener(
                  "scroll",
                  _e => {
                    setStartIndex(_prev =>
                      int_of_float(scrollTop(element) /. 200.)
                    );

                    setEndindex(_prev =>
                      int_of_float(
                        (
                          scrollTop(element)
                          +. float_of_int(
                               Webapi.Dom.HtmlElement.clientHeight(element),
                             )
                        )
                        /. 200.,
                      )
                    );
                  },
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

  let paddingBottom = (Belt.Array.length(data) - endIndex) * 200;

  <div ref={viewPortRef->ReactDOMRe.Ref.domRef}>
    <div
      className=Css.(
        style([
          paddingTop(px(startIndex * 200)),
          paddingBottom(px((Belt.Array.length(data) - endIndex) * 200)),
        ])
      )>
      <button
        className=Css.(
          style([display(block), marginLeft(auto), marginRight(auto)])
        )
        onClick={_e => setStartIndex(prevIndex => prevIndex + 1)}>
        {ReasonReact.string("Trigger rerender")}
      </button>
      Belt.Array.(
        data
        ->slice(startIndex, endIndex - startIndex + 5)
        ->map(item => (renderItem(item), identity(item)))
        ->map(itemTuple => {
            let (element, id) = itemTuple;

            ReasonReact.cloneElement(
              element,
              ~props={
                "ref": elementRef =>
                  Belt.HashMap.Int.set(
                    React.Ref.current(refMap),
                    id,
                    elementRef,
                  ),
              },
              [||],
            );
          })
      )
      ->ReasonReact.array
    </div>
  </div>;
};