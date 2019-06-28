[@react.component]
let make =
    (
      ~loading: bool,
      ~headerComponent,
      ~afterPadding: int,
      ~beforePadding: int,
      ~data,
      ~identity,
      ~onRefChange,
      ~renderItem,
      ~onReady,
    ) => {
  let (ready, setReady) = React.useState(() => false);

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

  React.useEffect1(
    () => {
      switch (afterPadding !== 0, ready) {
      | (true, false) =>
        onReady();
        setReady(_ => true);
      | (_, _) => ()
      };

      None;
    },
    [|afterPadding|],
  );

  <div
    className=Css.(
      style([beforePadding->px->paddingTop, afterPadding->px->paddingBottom])
    )>
    headerComponent
    elements->React.array
    {loading ? <div> "..."->React.string </div> : React.null}
  </div>;
};