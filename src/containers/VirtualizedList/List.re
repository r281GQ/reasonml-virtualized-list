[@react.component]
let make =
    (
      ~afterPadding: int,
      ~beforePadding: int,
      ~data,
      ~identity,
      ~onRefChange,
      ~renderItem,
      ~ready: bool,
    ) => {
  let elements =
    data
    ->Belt.Array.map(item => (renderItem(item), identity(item)))
    ->Belt.Array.map(((element, id)) =>
        ready
          ? ReasonReact.cloneElement(
              element,
              ~props={
                "key": id,
                "ref": elementRef => {
                  onRefChange(id, elementRef);
                },
              },
              [||],
            )
          : ReasonReact.cloneElement(
              element,
              ~props={
                "key": id,
                "style": {
                  "opacity": 0,
                },
                "ref": elementRef => {
                  onRefChange(id, elementRef);
                },
              },
              [||],
            )
      );

  <div
    className=Css.(
      style([beforePadding->px->paddingTop, afterPadding->px->paddingBottom])
    )>
    elements->React.array
  </div>;
};