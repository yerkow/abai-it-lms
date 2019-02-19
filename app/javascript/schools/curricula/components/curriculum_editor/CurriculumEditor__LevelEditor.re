open CurriculumEditor__Types;
open SchoolAdmin__Utils;

exception UnexpectedResponse(int);

let handleApiError =
  [@bs.open]
  (
    fun
    | UnexpectedResponse(code) => code
  );

let str = ReasonReact.string;

type keyForStateUpdation =
  | Name
  | UnlockOn;

type state = {
  name: string,
  levelNumber: int,
  unlockOn: option(string),
  saveDisabled: bool,
};

type action =
  | UpdateState(keyForStateUpdation, string)
  | UpdateLevelNumber(int);

let component = ReasonReact.reducerComponent("CurriculumEditor__LevelEditor");

let handleResponseJSON = json => {
  let id = json |> Json.Decode.(field("error", nullable(string))) |> Js.Null.toOption;
  switch (json |> Json.Decode.(field("error", nullable(string))) |> Js.Null.toOption) {
  | Some(error) => Notification.error("Something went wrong!!", error)
  | None => Notification.success("Success", "Target Created")
  };
  Js.log(id);
};

let handleResponseCB = json => Js.log(json);

let createLevel = (authenticityToken, course, state) => {
  let payload = Js.Dict.empty();
  let course_id = course |> Course.id |> string_of_int;

  Js.Dict.set(payload, "authenticity_token", authenticityToken |> Js.Json.string);
  Js.Dict.set(payload, "name", state.name |> Js.Json.string);
  Js.Dict.set(payload, "number", state.levelNumber |> string_of_int |> Js.Json.string);

  switch (state.unlockOn) {
  | Some(date) => Js.Dict.set(payload, "unlock_on", date |> Js.Json.string)
  | None => ()
  };

  let url = "/school/courses/" ++ course_id ++ "/levels";
  Api.create(url, payload, handleResponseCB);
};

let updateLevel = (authenticityToken, levelId, state) => {
  let payload = Js.Dict.empty();

  Js.Dict.set(payload, "authenticity_token", authenticityToken |> Js.Json.string);
  Js.Dict.set(payload, "name", state.name |> Js.Json.string);
  Js.Dict.set(payload, "number", state.levelNumber |> string_of_int |> Js.Json.string);

  switch (state.unlockOn) {
  | Some(date) => Js.Dict.set(payload, "unlock_on", date |> Js.Json.string)
  | None => ()
  };
  let url = "/school/levels/" ++ levelId;
  Api.update(url, payload, handleResponseCB);
};

let submitButton = (level, course, authenticityToken, state) =>
  switch (level |> Level.id) {
  | Some(id) =>
    <button
      disabled={state.saveDisabled}
      onClick={_event => updateLevel(authenticityToken, id |> string_of_int, state)}
      className="w-full bg-indigo-dark hover:bg-blue-dark text-white font-bold py-3 px-6 rounded focus:outline-none mt-3">
      {"Update Level" |> str}
    </button>
  | None =>
    <button
      onClick={_event => createLevel(authenticityToken, course, state)}
      className="w-full bg-indigo-dark hover:bg-blue-dark text-white font-bold py-3 px-6 rounded focus:outline-none mt-3">
      {"Create Level" |> str}
    </button>
  };

let make = (~level, ~course, ~authenticityToken, ~hideEditorActionCB, _children) => {
  ...component,
  initialState: () => {
    name: level |> Level.name,
    levelNumber: level |> Level.levelNumber,
    unlockOn: level |> Level.unlockOn,
    saveDisabled: true,
  },
  reducer: (action, state) =>
    switch (action) {
    | UpdateState(keyForStateUpdation, string) =>
      switch (keyForStateUpdation) {
      | Name => ReasonReact.Update({...state, name: string, saveDisabled: false})
      | UnlockOn => ReasonReact.Update({...state, unlockOn: Some(string), saveDisabled: false})
      }
    | UpdateLevelNumber(levelNumber) => ReasonReact.Update({...state, levelNumber, saveDisabled: false})
    },
  render: ({state, send}) => {
    let unlockOn =
      switch (state.unlockOn) {
      | Some(date) => date
      | None => ""
      };
    <div className="blanket">
      <div className="drawer-right">
        <div className="drawer-right-form w-full">
          <div className="w-full">
            <div className="mx-auto bg-white">
              <div className="max-w-md p-6 mx-auto">
                <h5 className="uppercase text-center border-b border-grey-light pb-2 mb-4">
                  {"Level Details" |> str}
                </h5>
                <label className="block tracking-wide text-grey-darker text-xs font-semibold mb-2" htmlFor="name">
                  {"Level Name*  " |> str}
                </label>
                <input
                  className="appearance-none block w-full bg-white text-grey-darker border border-grey-light rounded py-3 px-4 mb-6 leading-tight focus:outline-none focus:bg-white focus:border-grey"
                  id="name"
                  type_="text"
                  placeholder="Type level name here"
                  value={state.name}
                  onChange={event => send(UpdateState(Name, ReactEvent.Form.target(event)##value))}
                />
                <label className="block tracking-wide text-grey-darker text-xs font-semibold mb-2">
                  {"Level Number*  " |> str}
                </label>
                <input
                  className="appearance-none block w-full bg-white text-grey-darker border border-grey-light rounded py-3 px-4 mb-6 leading-tight focus:outline-none focus:bg-white focus:border-grey"
                  id="level number"
                  type_="number"
                  placeholder="Type level number here"
                  value={state.levelNumber |> string_of_int}
                  onChange={event => send(UpdateLevelNumber(ReactEvent.Form.target(event)##value))}
                />
                <label className="block tracking-wide text-grey-darker text-xs font-semibold mb-2">
                  {"Lock level*  " |> str}
                </label>
                <input
                  className="appearance-none block w-full bg-white text-grey-darker border border-grey-light rounded py-3 px-4 mb-6 leading-tight focus:outline-none focus:bg-white focus:border-grey"
                  id="level unlock date"
                  type_="date"
                  value=unlockOn
                  onChange={event => send(UpdateState(UnlockOn, ReactEvent.Form.target(event)##value))}
                />
                <div className="flex">
                  <button
                    onClick={_ => hideEditorActionCB()}
                    className="bg-indigo-dark hover:bg-blue-dark text-white font-bold py-3 px-6 rounded focus:outline-none mt-3">
                    {"Close" |> str}
                  </button>
                </div>
                <div className="flex"> {submitButton(level, course, authenticityToken, state)} </div>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>;
  },
};