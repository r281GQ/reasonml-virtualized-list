type keyProps = {. ref: string};

[@react.component]
let make =
    (
      ~afterPadding,
      ~beforePadding,
      ~data,
      ~identity,
      ~onRefChange,
      ~renderItem,
    ) => {
  <div
    className=Css.(
      style([beforePadding->px->paddingTop, afterPadding->px->paddingBottom])
    )>
    {data
     ->Belt.Array.map(item => (renderItem(item), identity(item)))
     ->Belt.Array.map(itemTuple => {
         let (element, id) = itemTuple;

         ReasonReact.cloneElement(
           element,
           ~props={"ref": elementRef => onRefChange(id, elementRef)},
           [||],
         );
       })
     ->ReasonReact.array}
  </div>;
};