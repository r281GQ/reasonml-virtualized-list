type keyProps = {. ref: string};

let scrollTop = Webapi.Dom.HtmlElement.scrollTop;

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
                let h = 0;

                // Js.log(Webapi.Dom.HtmlElement.clientHeight(e));
                // Js.log(Webapi.Dom.HtmlElement.scrollHeight(e));
                // Js.log(Webapi.Dom.HtmlElement.scrollTop(e));

                setStartIndex(_prev =>
                  int_of_float(scrollTop(element) /. 200.)
                );
              },
              element,
            )
          )
      );

      None;
    },
    [||],
  );

  Js.log(startIndex);

  Js.log(endIndex);

  <div ref={viewPortRef->ReactDOMRe.Ref.domRef}>
    <div className=Css.(style([paddingTop(px(0)), paddingBottom(px(0))]))>
      <button
        className=Css.(
          style([display(block), marginLeft(auto), marginRight(auto)])
        )
        onClick={_e => setStartIndex(prevIndex => prevIndex + 1)}>
        {ReasonReact.string("Trigger rerender")}
      </button>
      Belt.Array.(
        data
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