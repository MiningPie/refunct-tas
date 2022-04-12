static mut TAS_STATE = TasState {
    is_f_pressed: false,
    step_frame_mode: false,
    is_recording: false,
    is_replaying: Replaying::Nothing,
    recording: List::new(),
    events: List::new(),
    replay_index: 0,
    replay_keys_pressed: Set::new(),
};

struct TasState {
    is_f_pressed: bool,
    step_frame_mode: bool,
    is_recording: bool,
    is_replaying: Replaying,
    recording: List<RecordFrame>,
    events: List<Event>,
    replay_index: int,
    replay_keys_pressed: Set<int>,
}
struct RecordFrame {
    events: List<Event>,
    location: Location,
    rotation: Rotation,
    velocity: Velocity,
    acceleration: Acceleration,
}
enum Event {
    KeyPressed(int),
    KeyReleased(int),
    MouseMoved(int, int),
}
enum Replaying {
    Nothing,
    Inputs,
    Positions,
}

impl TasState {
    fn stop_replaying(self) {
        TAS_STATE.is_replaying = Replaying::Nothing;
        for code in TAS_STATE.replay_keys_pressed.values() {
            Tas::key_up(code, code, false);
        }
        TAS_STATE.replay_keys_pressed.clear();
    }
}

static TAS_COMPONENT = Component {
    draw_hud: fn(text: string) -> string {
        let text = f"{text}\nTAS: REQUIRES 60 FPS";
        let text = f"{text}\n     t toggle frame-step mode, f advance one frame";
        let text = f"{text}\n     r to record/stop, g to replay inputs, h to replay position";
        let mut text = f"{text}\n     Step-Frame: {TAS_STATE.step_frame_mode}    Recording: {TAS_STATE.is_recording}    Replay {TAS_STATE.is_replaying}: {TAS_STATE.replay_index}/{TAS_STATE.recording.len()}";

        if TAS_STATE.is_replaying == Replaying::Inputs {
            text = f"{text}\n\n";
            for key in TAS_STATE.replay_keys_pressed.values() {
                let key_string = if KEY_A.to_small() <= key && key <= KEY_Z.to_small() {
                    string::from_char(key)
                } else if key == KEY_LEFT_SHIFT.to_small() {
                    "SHIFT"
                } else if key == KEY_ESCAPE.to_small() {
                    "ESC"
                } else if key == KEY_SPACE.to_small() {
                    "SPACE"
                } else {
                    "?"
                };
                text = f"{text} {key_string}";
            }
            text = f"{text}\n";
        }

        text
    },
    tick_fn: Tas::yield,
    on_tick: fn() {
        // recording
        if TAS_STATE.is_recording {
            TAS_STATE.recording.push(RecordFrame {
                events: TAS_STATE.events,
                location: Tas::get_location(),
                rotation: Tas::get_rotation(),
                velocity: Tas::get_velocity(),
                acceleration: Tas::get_acceleration(),
            });
        }
        TAS_STATE.events = List::new();

        // replay
        if TAS_STATE.replay_index >= TAS_STATE.recording.len() && TAS_STATE.is_replaying != Replaying::Nothing {
            TAS_STATE.stop_replaying();
        }
        match TAS_STATE.is_replaying {
            Replaying::Nothing => (),
            Replaying::Inputs => {
                let frame = TAS_STATE.recording.get(TAS_STATE.replay_index).unwrap();
                if TAS_STATE.replay_index == 0 {
                    Tas::set_location(frame.location);
                    Tas::set_rotation(frame.rotation);
                    Tas::set_velocity(frame.velocity);
                    Tas::set_acceleration(frame.acceleration);
                }
                for event in frame.events {
                    match event {
                        Event::KeyPressed(code) => {
                            TAS_STATE.replay_keys_pressed.insert(code);
                            Tas::key_down(code, code, false);
                        },
                        Event::KeyReleased(code) => {
                            TAS_STATE.replay_keys_pressed.remove(code);
                            Tas::key_up(code, code, false);
                        },
                        Event::MouseMoved(x, y) => {
                            Tas::move_mouse(x, y);
                        },
                    }
                }
                TAS_STATE.replay_index += 1;
            },
            Replaying::Positions => {
                let frame = TAS_STATE.recording.get(TAS_STATE.replay_index).unwrap();
                Tas::set_location(frame.location);
                Tas::set_rotation(frame.rotation);
                Tas::set_velocity(frame.velocity);
                Tas::set_acceleration(frame.acceleration);
                TAS_STATE.replay_index += 1;
            }
        }
    },
    on_yield: fn() {
        if !TAS_STATE.step_frame_mode || TAS_STATE.is_f_pressed {
            step_frame(Option::Some(1./60.), Tas::step);
        }
    },
    on_new_game: fn() {},
    on_level_change: fn(old: int, new: int) {},
    on_reset: fn(old: int, new: int) {},
    on_platforms_change: fn(old: int, new: int) {},
    on_buttons_change: fn(old: int, new: int) {},
    on_key_down: fn(key_code: KeyCode, is_repeat: bool) {
        let key = key_code.to_small();
        if key == KEY_T.to_small() {
            TAS_STATE.step_frame_mode = !TAS_STATE.step_frame_mode;
        } else if key == KEY_R.to_small() {
            TAS_STATE.is_recording = !TAS_STATE.is_recording;
            if TAS_STATE.is_recording {
                TAS_STATE.recording = List::new();
            }
        } else if key == KEY_G.to_small() {
            if TAS_STATE.is_replaying == Replaying::Inputs {
                TAS_STATE.stop_replaying();
            } else {
                TAS_STATE.is_replaying = Replaying::Inputs;
                TAS_STATE.replay_index = 0;
            }
        } else if key == KEY_H.to_small() {
            if TAS_STATE.is_replaying == Replaying::Positions {
                TAS_STATE.is_replaying = Replaying::Nothing;
            } else {
                TAS_STATE.is_replaying = Replaying::Positions;
                TAS_STATE.replay_index = 0;
            }
        } else if key == KEY_F.to_small() {
            if is_repeat {
                TAS_STATE.is_f_pressed = true;
            } else {
                step_frame(Option::Some(1./60.), Tas::step);
            }
        } else if !is_repeat {
            TAS_STATE.events.push(Event::KeyPressed(key_code.large_value));
        }
    },
    on_key_up: fn(key_code: KeyCode) {
        let key = key_code.to_small();

        if key == KEY_F.to_small() {
            TAS_STATE.is_f_pressed = false;
        } else if key == KEY_T.to_small() || key == KEY_R.to_small() || key == KEY_G.to_small() || key == KEY_H.to_small() {
            // pass
        } else {
            TAS_STATE.events.push(Event::KeyReleased(key_code.large_value));
        }
    },
    on_mouse_move: fn(x: int, y: int) {
        TAS_STATE.events.push(Event::MouseMoved(x, y));
    },
    on_component_exit: fn() {},
};