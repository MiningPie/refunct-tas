struct Settings {
    ui_scale: float,
    show_character_stats: bool,
    show_game_stats: bool,
    multiplayer_name: string,
}

static mut SETTINGS = Settings::load();

impl Settings {
    fn load() -> Settings {
        let settings = Tas::load_settings();
        let map = settings.unwrap_or(Map::new());
        let ui_scale = match map.get("ui_scale") {
            Option::Some(scale) => {
                let list = scale.split("\\.");
                let num = list.get(0).unwrap().parse_int().unwrap();
                let decimal = list.get(1).unwrap().parse_int().unwrap();
                num.to_float() + decimal.to_float() / 10.
            },
            Option::None => 2.,
        };
        let show_character_stats = match map.get("show_character_stats") {
             Option::Some(char_stats) => char_stats == "true",
             Option::None => false,
        };
        let show_game_stats = match map.get("show_game_stats") {
             Option::Some(game_stats) => game_stats == "true",
             Option::None => false,
        };
        let multiplayer_name = match map.get("multiplayer_name") {
             Option::Some(multiplayer_name) => multiplayer_name,
             Option::None => "Player",
        };
        Settings {
            ui_scale: ui_scale,
            show_character_stats: show_character_stats,
            show_game_stats: show_game_stats,
            multiplayer_name: multiplayer_name,
        }
    }

    fn store(self) {
        let map = Map::new();
        map.insert("ui_scale", f"{self.ui_scale:?}");
        map.insert("show_character_stats", f"{self.show_character_stats}");
        map.insert("show_game_stats", f"{self.show_game_stats}");
        map.insert("multiplayer_name", f"{self.multiplayer_name}");
        Tas::store_settings(map);
    }

    fn increase_ui_scale(mut self) {
        self.ui_scale += 0.5;
        self.ui_scale = self.ui_scale.min(10.);
        self.store();
    }
    fn decrease_ui_scale(mut self) {
        self.ui_scale -= 0.5;
        self.ui_scale = self.ui_scale.max(0.5);
        self.store();
    }
    fn toggle_show_character_stats(mut self) {
        self.show_character_stats = !self.show_character_stats;
        self.store();
    }
    fn toggle_show_game_stats(mut self) {
        self.show_game_stats = !self.show_game_stats;
        self.store();
    }
    fn set_multiplayer_name(mut self, name: string) {
        print(f"1:   name: {name}\nself.multiplayer_name: {self.multiplayer_name}\nSETTINGS.multiplayer_name: {SETTINGS.multiplayer_name}");
        SETTINGS.multiplayer_name = name;
        print(f"2:   name: {name}\nself.multiplayer_name: {self.multiplayer_name}\nSETTINGS.multiplayer_name: {SETTINGS.multiplayer_name}");
        self.multiplayer_name = name;
        print(f"3:   name: {name}\nself.multiplayer_name: {self.multiplayer_name}\nSETTINGS.multiplayer_name: {SETTINGS.multiplayer_name}");
        self.store();
        print(f"4:   name: {name}\nself.multiplayer_name: {self.multiplayer_name}\nSETTINGS.multiplayer_name: {SETTINGS.multiplayer_name}");
    }
}
