type dataItemType = {
  id: int,
  sampleString: string,
};

let data =
  Belt.Array.(
    range(0, 100)->map(id => {id, sampleString: string_of_int(id)})
  );

[@react.component]
let make = () => {
  let testRef = React.useRef(Js.Nullable.null);

  <div>
    <VirtualizedList
      data
      renderItem={data =>
        <Item
          key={data.id->string_of_int}
          id={data.id}
          ref={testRef->ReactDOMRe.Ref.domRef}
        />
      }
      identity={data => data.id}
    />
  </div>;
};