[@react.component]
let make =
  React.forwardRef((~id: int, ~heightProp: int, ref_) =>
    <div
      ref=?{
        ref_->Js.Nullable.toOption->Belt.Option.map(ReactDOMRe.Ref.domRef)
      }
      className=Css.(
        style([
          borderRadius(px(8)),
          display(`flex),
          flexDirection(row),
          justifyContent(center),
          // marginBottom(px(8)),
          // marginTop(px(8)),
          backgroundColor(id mod 2 === 0 ? hex("793698") : hex("d6b4e7")),
          height(px(heightProp)),
        ])
      )>
      <div
        className=Css.(
          style([
            display(`flex),
            flexDirection(column),
            justifyContent(center),
            fontSize(px(20)),
          ])
        )>
        {id->string_of_int->ReasonReact.string}
      </div>
    </div>
  );