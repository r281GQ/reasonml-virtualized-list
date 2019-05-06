type dataItemType = {
  id: int,
  sampleString: string,
};

let data =
  Belt.Array.(
    range(0, 25)->map(id => {id, sampleString: string_of_int(id)})
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
        <Item randomise=true key={data.id->string_of_int} id={data.id} />
      }
      identity={data => data.id}
    />
  </div>;
};