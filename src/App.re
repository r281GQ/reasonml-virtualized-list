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
    range(0, 99)
    ->map(id =>
        {
          id,
          heightProp: unit->random->randomHeight->int_of_float,
          sampleString: id->string_of_int,
        }
      )
  );

module List = {
  [@react.component]
  let make = (~scrollPosition, ~setScrollPosition, ~heightMap, ~setHeightMap) => {
    let testRef = React.useRef(Js.Nullable.null);

    <div
      ref={testRef->ReactDOMRe.Ref.domRef}
      className=Css.(style([maxHeight(vh(90.)), overflowY(`scroll)]))>
      <VirtualizedList
        viewPortRef=testRef
        onDestroy={(~scrollPosition, ~heightMap) => {
          Js.log(scrollPosition);
          Js.log(heightMap);

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
    // switch (url.path) {
    // | []
    // | ["list"] =>
    //   <div>
    //     <button onClick={_e => ReasonReactRouter.push("other")}>
    //       {React.string("To other route")}
    //     </button>
    //   </div>
    // | ["other"] =>
    //   <div>
    //     <button onClick={_e => ReasonReactRouter.push("list")}>
    //       {React.string("To list")}
    //     </button>
    //   </div>
    // | [a] => <div> {React.string("On route " ++ a)} </div>
    // | _ => <div> {React.string("Nope!")} </div>
    // };
    };
  };
};

[@react.component]
let make = () => {
  let {path}: ReasonReactRouter.url = ReasonReactRouter.useUrl();

  let (tempRoute, changeTempRoute) = React.useState(() => Home);

  let (scrollPosition, setScrollPosition) = React.useState(() => 0);

  let (heightMap, setHeightMap) =
    React.useState(() => Belt.HashMap.Int.make(1));

  let currentPath = path->Belt.List.head;

  <React.Fragment>
    <Header tempRoute changeTempRoute={route => changeTempRoute(_ => route)} />
    // {switch (path) {
    //  | []
    //  | ["list"] =>
    //    <List scrollPosition setScrollPosition heightMap setHeightMap />
    //  | ["other"] => <div> "Other route!"->React.string </div>
    //  | _ => <div> "Nope!"->React.string </div>
    //  }}
    {switch (tempRoute) {
     | Home => <List scrollPosition setScrollPosition heightMap setHeightMap />
     | Other => <div> "Other route!"->React.string </div>
     }}
  </React.Fragment>;
};