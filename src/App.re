[@bs.scope "Math"] [@bs.val] external random: unit => float = "random";

type dataItemType = {
  id: int,
  heightProp: int,
  sampleString: string,
};

let unit = ();

let randomHeight = x => x *. 100. +. 200.;

let data =
  Belt.Array.(
    range(0, 25)
    ->map(id =>
        {
          id,
          heightProp: unit->random->randomHeight->int_of_float,
          sampleString: id->string_of_int,
        }
      )
  );

[@react.component]
let make = () => {
  let testRef = React.useRef(Js.Nullable.null);

  <div
    ref={testRef->ReactDOMRe.Ref.domRef}
    className=Css.(style([maxHeight(vh(100.)), overflowY(`scroll)]))>
    <VirtualizedList
      viewPortRef=testRef
      data
      renderItem={data =>
        <Item
          heightProp={data.heightProp}
          key={data.id->string_of_int}
          id={data.id}
        />
      }
      identity={data => data.id}
    />
  </div>;
};