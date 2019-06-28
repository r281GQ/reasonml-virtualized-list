[@bs.scope "Math"] [@bs.val] external random: unit => float = "random";

type dataItemType = {
  id: int,
  heightProp: int,
  sampleString: string,
};

let unit = ();

let randomHeight = x => x *. 150. +. 200.;

let data =
  Belt.Array.(
    range(1, 100)
    ->map(id =>
        {
          id,
          heightProp: unit->random->randomHeight->int_of_float,
          sampleString: id->string_of_int,
        }
      )
  );

module VList = {
  [@react.component]
  let make = (~scrollPosition, ~setScrollPosition, ~heightMap, ~setHeightMap) => {
    let testRef = React.useRef(Js.Nullable.null);
    let (refreshing, setRefreshing) = React.useState(() => true);

    React.useEffect1(
      () => {
        Js.Global.setTimeout(() => setRefreshing(_ => false), 1000)->ignore;

        None;
      },
      [||],
    );

    <div
      ref={testRef->ReactDOMRe.Ref.domRef}
      className=Css.(style([maxHeight(vh(90.)), overflowY(`scroll)]))>
      <VirtualizedList
        refreshingComponent={<div> "..."->React.string </div>}
        refreshing
        viewPortRef=testRef
        defaultPosition={Some(scrollPosition)}
        defaultHeightMap={Some(heightMap)}
        onDestroy={(~scrollPosition, ~heightMap) => {
          setScrollPosition(_ => scrollPosition);
          setHeightMap(_ => heightMap);
        }}
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
};

type tempRouteState =
  | Home
  | Other;

module Header = {
  [@react.component]
  let make = (~tempRoute, ~changeTempRoute) => {
    // let url = ReasonReactRouter.useUrl();
    switch (tempRoute) {
    | Home =>
      <div>
        <button onClick={_e => changeTempRoute(Other)}>
          {React.string("To other route")}
        </button>
      </div>
    | Other =>
      <div>
        <button onClick={_e => changeTempRoute(Home)}>
          {React.string("To list")}
        </button>
      </div>
    };
  };
};

[@react.component]
let make = () => {
  let (tempRoute, changeTempRoute) = React.useState(() => Home);

  let (scrollPosition, setScrollPosition) = React.useState(() => 0);

  let (heightMap, setHeightMap) =
    React.useState(() => Belt.HashMap.Int.make(~hintSize=1));

  <React.Fragment>
    <Header tempRoute changeTempRoute={route => changeTempRoute(_ => route)} />
    {switch (tempRoute) {
     | Home =>
       <VList scrollPosition setScrollPosition heightMap setHeightMap />
     | Other => <div> "Other route!"->React.string </div>
     }}
  </React.Fragment>;
};