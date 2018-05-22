// (c) 2016 - Present, Bracket Productions

#include "NRaws.h"

NRaws::NRaws()
{
}

NRaws::~NRaws()
{
	ExitLua();
}

void NRaws::LoadRaws() {
	using namespace string_tables;
	string_tables.Empty();
	FString raws_path = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
	
	// Load string tables for first names and last names
	LoadStringTable(FIRST_NAMES_MALE, raws_path + FString("first_names_male.txt"));
	LoadStringTable(FIRST_NAMES_FEMALE, raws_path + FString("first_names_female.txt"));
	LoadStringTable(LAST_NAMES, raws_path + FString("last_names.txt"));
	LoadStringTable(NEW_ARRIVAL_QUIPS, raws_path + FString("newarrival.txt"));
	LoadStringTable(MENU_SUBTITLES, raws_path + FString("menu_text.txt"));

	// Setup Lua
	InitLua();

	// Load the game
	LoadStringTable(-1, raws_path + FString("index.txt"));
	for (const auto &filename : string_tables[-1].strings) {
		LoadLuaScript(raws_path + filename);
	}

	LoadGameTables();
}

void NRaws::LoadGameTables() {
	// TODO
	ReadMaterialTypes();

	ReadClothing();
	ReadLifeEvents();
	ReadProfessions();
	ReadStockpiles();
	ReadItems();
	ReadBuildings();
	ReadReactions();
	ReadPlantTypes();
	//read_biome_types();
	//read_biome_textures();
	//read_species_types();
	//read_creature_types();
}

void NRaws::InitLua() {
	lua_state = luaL_newstate();

	// load Lua libraries
	static const luaL_Reg lualibs[] =
	{
		{ "base", luaopen_base },
		{ NULL, NULL }
	};

	const luaL_Reg *lib = lualibs;
	for (; lib->func != NULL; lib++)
	{
		lib->func(lua_state);
		lua_settop(lua_state, 0);
	}
}

void NRaws::ExitLua() {
	lua_close(lua_state);
}

void NRaws::LoadLuaScript(const FString &filename) {
	char * path_c_str = TCHAR_TO_ANSI(*filename);
	if (luaL_loadfile(lua_state, path_c_str) || lua_pcall(lua_state, 0, 0, 0)) {
		//glog(log_target::LOADER, log_severity::error, "Lua error ({0}): {1}", filename, lua_tostring(lua_state, -1));
		//std::cout << "Lua Error: " << filename << " - " << lua_tostring(lua_state, -1) << "\n";
	}
}

void NRaws::ReadMaterialTypes() {
	lua_getglobal(lua_state, "materials");
	lua_pushnil(lua_state);

	while (lua_next(lua_state, -2) != 0)
	{
		FString key = lua_tostring(lua_state, -2);

		material_def_t m;
		m.tag = key;

		lua_pushstring(lua_state, TCHAR_TO_ANSI(*key));
		lua_gettable(lua_state, -2);
		while (lua_next(lua_state, -2) != 0) {
			FString field = lua_tostring(lua_state, -2);

			if (field == "name") m.name = lua_tostring(lua_state, -1);
			if (field == "type") {
				const FString type_s = lua_tostring(lua_state, -1);
				if (type_s == "cluster_rock") {
					m.spawn_type = material_def_spawn_type_t::CLUSTER_ROCK;
				}
				else if (type_s == "rock") {
					m.spawn_type = material_def_spawn_type_t::ROCK;
				}
				else if (type_s == "soil") {
					m.spawn_type = material_def_spawn_type_t::SOIL;
				}
				else if (type_s == "sand") {
					m.spawn_type = material_def_spawn_type_t::SAND;
				}
				else if (type_s == "metal") {
					m.spawn_type = material_def_spawn_type_t::METAL;
				}
				else if (type_s == "synthetic") {
					m.spawn_type = material_def_spawn_type_t::SYNTHETIC;
				}
				else if (type_s == "organic") {
					m.spawn_type = material_def_spawn_type_t::ORGANIC;
				}
				else if (type_s == "leather") {
					m.spawn_type = material_def_spawn_type_t::LEATHER;
				}
				else if (type_s == "food") {
					m.spawn_type = material_def_spawn_type_t::FOOD;
				}
				else if (type_s == "spice") {
					m.spawn_type = material_def_spawn_type_t::SPICE;
				}
				else if (type_s == "blight") {
					m.spawn_type = material_def_spawn_type_t::BLIGHT;
				}
				else {
					//glog(log_target::LOADER, log_severity::warning, "WARNING: Unknown material type: {0}", type_s);
				}

			}
			if (field == "parent") m.parent_material_tag = lua_tostring(lua_state, -1);
			if (field == "glyph") m.glyph = static_cast<uint16_t>(lua_tonumber(lua_state, -1));
			if (field == "fg") m.fg = ReadLuaColor("fg");
			if (field == "bg") m.bg = ReadLuaColor("bg");
			if (field == "hit_points") m.hit_points = static_cast<uint8_t>(lua_tonumber(lua_state, -1));
			if (field == "mines_to") m.mines_to_tag = lua_tostring(lua_state, -1);
			if (field == "mines_to_also") m.mines_to_tag_second = lua_tostring(lua_state, -1);
			if (field == "layer") m.layer = lua_tostring(lua_state, -1);
			if (field == "ore_materials") {
				lua_pushstring(lua_state, TCHAR_TO_ANSI(*field));
				lua_gettable(lua_state, -2);
				while (lua_next(lua_state, -2) != 0) {
					const FString metal_name = lua_tostring(lua_state, -1);
					m.ore_materials.Emplace(metal_name);
					lua_pop(lua_state, 1);
				}
			}
			if (field == "damage_bonus") m.damage_bonus = static_cast<int>(lua_tonumber(lua_state, -1));
			if (field == "ac_bonus") m.ac_bonus = static_cast<float>(lua_tonumber(lua_state, -1));
			if (field == "FloorRough") m.floor_rough = lua_tostring(lua_state, -1);
			if (field == "FloorSmooth") m.floor_smooth = lua_tostring(lua_state, -1);
			if (field == "WallRough") m.wall_rough = lua_tostring(lua_state, -1);
			if (field == "WallSmooth") m.wall_smooth = lua_tostring(lua_state, -1);
			lua_pop(lua_state, 1);
		}
		material_defs.Emplace(m);

		lua_pop(lua_state, 1);
	}

	material_defs.Sort([](material_def_t a, material_def_t b) {
		return a.tag < b.tag;
	});

	texture_atlas.Empty();
	material_defs_idx.Empty();
	int next_id = 0;
	for (std::size_t material_index = 0; material_index < material_defs.Num(); ++material_index) {
		material_defs_idx[material_defs[material_index].tag] = material_index;

		material_defs[material_index].floor_rough_id = next_id;
		material_defs[material_index].floor_smooth_id = next_id + 1;
		material_defs[material_index].wall_rough_id = next_id + 2;
		material_defs[material_index].wall_smooth_id = next_id + 3;

		texture_atlas[next_id] = material_defs[material_index].floor_rough;
		texture_atlas[next_id + 1] = material_defs[material_index].floor_smooth;
		texture_atlas[next_id + 2] = material_defs[material_index].wall_rough;
		texture_atlas[next_id + 3] = material_defs[material_index].wall_smooth;

		next_id += 4;
	}
}

void NRaws::ReadClothing() {
	FString tag;
	clothing_t c;
	read_lua_table("clothing",
		[&c, &tag](const auto &key) { tag = key; c = clothing_t{}; },
		[&c, this](const auto &key) {
			clothing_types[key] = c;
		},
		lua_parser(
			std::initializer_list<lua_parser_inner_t>{
				{ "name",        [&c,this]() { c.name = this->lua_str(); } },
				{ "slot",        [&c,this]() { c.slot = this->lua_str(); } },
				{ "description", [&c,this]() { c.description = this->lua_str(); } },
				{ "ac",          [&c,this]() { c.armor_class = this->lua_float(); } },
				{ "glyph",       [&c,this]() { c.clothing_glyph = this->lua_int(); } },
				{ "layer",       [&c,this]() { c.clothing_layer = this->lua_int(); } },
				{ "vox",         [&c,this]() { c.voxel_model = this->lua_int(); } },
				{ "colors",      [&c,this]() { read_lua_table_inner("colors", [&c](auto col) { c.colors.Emplace(col); }); } } 
			}
		) // End lua-parser
	);
}

void NRaws::ReadLifeEvents() {
	FString tag;
	life_event_template le;
	read_lua_table("life_events",
		[&le, &tag](const auto &key) { tag = key; le = life_event_template{}; },
		[&le, &tag, this](const auto &key) { life_event_defs[tag] = le; },
		lua_parser(
			std::initializer_list<lua_parser_inner_t>{
				{ "min_age", [&le,this]() { le.min_age = lua_int(); } },
				{ "max_age", [&le,this]() { le.max_age = lua_int(); } },
				{ "weight", [&le,this]() { le.weight = lua_int(); } },
				{ "description", [&le,this]() { le.description = lua_str(); } },
				{ "modifiers", [&le,this]() { read_stat_modifiers("modifiers", le); } },
				{ "skills",      [&le,this]() {
					read_lua_table_inner("skills", [&le](auto s) { le.skills.Emplace(s); });
				} },
				{ "requires",      [&le,this]() {
					read_lua_table_inner("requires", [&le](auto s) { le.requires_event.Emplace(s); });
				} },
				{ "precludes",      [&le,this]() {
					read_lua_table_inner("precludes", [&le](auto s) { le.precludes_event.Emplace(s); });
				} }
			}
		)
	);
}

void NRaws::ReadProfessions() {
	FString tag;
	profession_t p;

	read_lua_table("starting_professions",
		[&p, &tag](const auto &key) { tag = key; p = profession_t{}; },
		[&p, this](const auto &key) { starting_professions.Emplace(p); },
		lua_parser(
			std::initializer_list<lua_parser_inner_t>{
				{ "name", [&p,this]() { p.name = lua_str(); } },
				{ "modifiers", [&p,this]() {
					read_stat_modifiers("modifiers", p);
				} },
				{ "clothing", [&p,this]() {
					lua_pushstring(lua_state, "clothing");
					lua_gettable(lua_state, -2);
					while (lua_next(lua_state, -2) != 0) {
						const FString gender_specifier = lua_tostring(lua_state, -2);
						lua_pushstring(lua_state, TCHAR_TO_ANSI(*gender_specifier));
						lua_gettable(lua_state, -2);
						while (lua_next(lua_state, -2) != 0) {
							const FString slot = lua_tostring(lua_state, -2);
							const FString item = lua_tostring(lua_state, -1);
							//std::cout << p.name << " - " << slot << ", " << item << "\n";
							int gender_tag = 0;
							if (gender_specifier == "male") gender_tag = 1;
							if (gender_specifier == "female") gender_tag = 2;
							p.starting_clothes.Emplace(TTuple<uint8_t, FString, FString>(gender_tag, slot, item));
							lua_pop(lua_state, 1);
						}
						lua_pop(lua_state, 1);
					}
				} }
			}
		)
	);
}

void NRaws::ReadStockpiles() {
	FString tag;
	FString name;
	stockpile_def_t c;

	read_lua_table("stockpiles",
		[&tag, &c](const auto &key) { tag = key; c = stockpile_def_t{}; c.tag = key; },
		[&c,this](const auto &key) { stockpile_defs[c.index] = c; if (c.tag == "clothing") clothing_stockpile = c.index; },
		lua_parser(
			std::initializer_list<lua_parser_inner_t>{
				{ "name", [&c,this]() { c.name = lua_str(); } },
				{ "id", [&c,this]() { c.index = lua_int(); } }
			}
		)
	);
}

void NRaws::ReadItems() {
	FString tag;
	item_def_t c;

	read_lua_table("items",
		[&c, &tag](const auto &key) { tag = key; c = item_def_t{}; c.tag = tag; },
		[&c, &tag, this](const auto &key) { item_defs[tag] = c; },
			lua_parser(
				std::initializer_list<lua_parser_inner_t>{
					{ "name", [&c,this]() { c.name = lua_str(); } },
					{ "description", [&c,this]() { c.description = lua_str(); } },
					{ "background", [&c,this]() { c.bg = ReadLuaColor("background"); } },
					{ "foreground", [&c,this]() { c.bg = ReadLuaColor("foreground"); } },
					{ "glyph", [&c,this]() { c.glyph = lua_int(); } },
					{ "glyph_ascii", [&c,this]() { c.glyph_ascii = lua_int(); } },
					{ "damage_n", [&c,this]() { c.damage_n = lua_int(); } },
					{ "damage_d", [&c,this]() { c.damage_d = lua_int(); } },
					{ "damage_mod", [&c,this]() { c.damage_mod = lua_int(); } },
					{ "damage_stat", [&c,this]() { c.damage_stat = lua_str(); } },
					{ "ammo", [&c,this]() { c.ammo = lua_str(); } },
					{ "range", [&c,this]() { c.range = lua_int(); } },
					{ "vox", [&c,this]() { c.voxel_model = lua_int(); } },
					{ "clothing_layer", [&c,this]() { c.clothing_layer = lua_int(); } },
					{ "stack_size", [&c,this]() { c.stack_size = lua_int(); } },
					{ "initiative_penalty", [&c,this]() { c.initiative_penalty = lua_int(); } },
					{ "stockpile", [&c,this]() { c.stockpile_idx = lua_int(); } },
					{ "itemtype", [&c,this]() {
						read_lua_table_inner("itemtype", [&c](auto type) {
							if (type == "component") c.categories[COMPONENT] = true;
							if (type == "tool-chopping") c.categories[TOOL_CHOPPING] = true;
							if (type == "tool-digging") c.categories[TOOL_DIGGING] = true;
							if (type == "weapon-melee") c.categories[WEAPON_MELEE] = true;
							if (type == "weapon-ranged") c.categories[WEAPON_RANGED] = true;
							if (type == "ammo") c.categories[WEAPON_AMMO] = true;
							if (type == "food") c.categories[ITEM_FOOD] = true;
							if (type == "spice") c.categories[ITEM_SPICE] = true;
							if (type == "drink") c.categories[ITEM_DRINK] = true;
							if (type == "hide") c.categories[ITEM_HIDE] = true;
							if (type == "bone") c.categories[ITEM_BONE] = true;
							if (type == "skull") c.categories[ITEM_SKULL] = true;
							if (type == "leather") c.categories[ITEM_LEATHER] = true;
							if (type == "tool-farming") c.categories[ITEM_FARMING] = true;
							if (type == "seed") c.categories[ITEM_SEED] = true;
							if (type == "topsoil") c.categories[ITEM_TOPSOIL] = true;
							if (type == "fertilizer") c.categories[ITEM_FERTILIZER] = true;
							if (type == "food-prepared") c.categories[ITEM_FOOD_PREPARED] = true;
						});
				} }
			}
		)
	);
}

void NRaws::ReadBuildings() {
	lua_getglobal(lua_state, "buildings");
	lua_pushnil(lua_state);

	while (lua_next(lua_state, -2) != 0)
	{
		building_def_t c;

		FString key = lua_tostring(lua_state, -2);
		c.tag = key;
		c.hashtag = GetTypeHash(c.tag);

		lua_pushstring(lua_state, TCHAR_TO_ANSI(*key));
		lua_gettable(lua_state, -2);
		while (lua_next(lua_state, -2) != 0) {
			FString field = lua_tostring(lua_state, -2);

			if (field == "name") c.name = lua_tostring(lua_state, -1);
			if (field == "description") c.description = lua_tostring(lua_state, -1);
			if (field == "blocked") c.blocked = lua_tostring(lua_state, -1);
			if (field == "vox") c.vox_model = static_cast<int>(lua_tonumber(lua_state, -1));
			if (field == "structure") c.structure = true;
			if (field == "emits_smoke") c.emits_smoke = lua_toboolean(lua_state, -1);
			if (field == "components") {
				lua_pushstring(lua_state, TCHAR_TO_ANSI(*field));
				lua_gettable(lua_state, -2);
				while (lua_next(lua_state, -2) != 0) {

					lua_pushnil(lua_state);
					lua_gettable(lua_state, -2);

					reaction_input_t comp;
					comp.required_material = 0;
					comp.required_material_type = material_def_spawn_type_t::NO_SPAWN_TYPE;
					while (lua_next(lua_state, -2) != 0) {
						FString f = lua_tostring(lua_state, -2);

						if (f == "item") comp.tag = lua_tostring(lua_state, -1);
						if (f == "qty") comp.quantity = static_cast<int>(lua_tonumber(lua_state, -1));
						if (f == "material") {
							const FString mat_name = lua_tostring(lua_state, -1);
							const auto matfinder = get_material_by_tag(mat_name);
							if (matfinder == 0) {
								//glog(log_target::LOADER, log_severity::warning, "WARNING: Reaction %s references unknown material {0}", c.name, mat_name);
							}
							else {
								comp.required_material = matfinder;
							}
						}
						if (f == "mat_type") {
							const FString type_s = lua_tostring(lua_state, -1);
							if (type_s == "cluster_rock") {
								comp.required_material_type = material_def_spawn_type_t::CLUSTER_ROCK;
							}
							else if (type_s == "rock") {
								comp.required_material_type = material_def_spawn_type_t::ROCK;
							}
							else if (type_s == "soil") {
								comp.required_material_type = material_def_spawn_type_t::SOIL;
							}
							else if (type_s == "sand") {
								comp.required_material_type = material_def_spawn_type_t::SAND;
							}
							else if (type_s == "metal") {
								comp.required_material_type = material_def_spawn_type_t::METAL;
							}
							else if (type_s == "synthetic") {
								comp.required_material_type = material_def_spawn_type_t::SYNTHETIC;
							}
							else if (type_s == "organic") {
								comp.required_material_type = material_def_spawn_type_t::ORGANIC;
							}
							else if (type_s == "leather") {
								comp.required_material_type = material_def_spawn_type_t::LEATHER;
							}
							else {
								//glog(log_target::LOADER, log_severity::warning, "Unknown material type {0}", type_s);
							}
						}
						lua_pop(lua_state, 1);
					}

					c.components.Emplace(comp);
					lua_pop(lua_state, 1);
				}
			}
			if (field == "skill") {
				lua_pushstring(lua_state, TCHAR_TO_ANSI(*field));
				lua_gettable(lua_state, -2);
				while (lua_next(lua_state, -2) != 0) {
					const FString type = lua_tostring(lua_state, -2);
					if (type == "name") c.skill.Key = lua_tostring(lua_state, -1);
					if (type == "difficulty") c.skill.Value = static_cast<int>(lua_tonumber(lua_state, -1));
					lua_pop(lua_state, 1);
				}
			}
			if (field == "provides") {
				lua_pushstring(lua_state, TCHAR_TO_ANSI(*field));
				lua_gettable(lua_state, -2);

				while (lua_next(lua_state, -2) != 0) {
					building_provides_t provisions;
					FString type = lua_tostring(lua_state, -2);
					if (type == "table") provisions.provides = provides_desk;
					if (type == "wall") provisions.provides = provides_wall;
					if (type == "door") provisions.provides = provides_door;
					if (type == "food") provisions.provides = provides_food;
					if (type == "sleep") provisions.provides = provides_sleep;
					if (type == "floor") provisions.provides = provides_floor;
					if (type == "stairs_up") provisions.provides = provides_stairs_up;
					if (type == "stairs_down") provisions.provides = provides_stairs_down;
					if (type == "stairs_updown") provisions.provides = provides_stairs_updown;
					if (type == "ramp") provisions.provides = provides_ramp;
					if (type == "light") provisions.provides = provides_light;
					if (type == "cage_trap") provisions.provides = provides_cage_trap;
					if (type == "stonefall_trap") provisions.provides = provides_stonefall_trap;
					if (type == "blade_trap") provisions.provides = provides_blades_trap;
					if (type == "spike_trap") provisions.provides = provides_spikes;
					if (type == "lever") provisions.provides = provides_lever;
					if (type == "pressure_plate") provisions.provides = provides_pressure_plate;
					if (type == "signal") provisions.provides = provides_signal_recipient;
					if (type == "storage") provisions.provides = provides_storage;
					if (type == "oscillator") provisions.provides = provides_oscillator;
					if (type == "gate_and") provisions.provides = provides_and_gate;
					if (type == "gate_or") provisions.provides = provides_or_gate;
					if (type == "gate_not") provisions.provides = provides_not_gate;
					if (type == "gate_nand") provisions.provides = provides_nand_gate;
					if (type == "gate_nor") provisions.provides = provides_nor_gate;
					if (type == "gate_xor") provisions.provides = provides_xor_gate;
					if (type == "float_sensor") provisions.provides = provides_float_gauge;
					if (type == "proximity_sensor") provisions.provides = provides_proximity_sensor;
					if (type == "support") provisions.provides = provides_support;

					lua_pushstring(lua_state, TCHAR_TO_ANSI(*type));
					lua_gettable(lua_state, -2);
					while (lua_next(lua_state, -2) != 0) {
						const FString inner_type = lua_tostring(lua_state, -2);
						if (inner_type == "energy_cost") provisions.energy_cost = static_cast<int>(lua_tonumber(lua_state, -1));
						if (inner_type == "radius") provisions.radius = static_cast<int>(lua_tonumber(lua_state, -1));
						if (inner_type == "color") provisions.color = ReadLuaColor("color");
						if (inner_type == "alternate_vox") provisions.alternate_vox = static_cast<int>(lua_tonumber(lua_state, -1));
						lua_pop(lua_state, 1);
					}

					c.provides.Emplace(provisions);
					lua_pop(lua_state, 1);
				}
			}
			if (field == "render") {
				lua_pushstring(lua_state, TCHAR_TO_ANSI(*field));
				lua_gettable(lua_state, -2);
				while (lua_next(lua_state, -2) != 0) {
					FString type = lua_tostring(lua_state, -2);
					if (type == "width") c.width = static_cast<int>(lua_tonumber(lua_state, -1));
					if (type == "height") c.height = static_cast<int>(lua_tonumber(lua_state, -1));
					if (type == "tiles") {
						/*
						lua_pushstring(lua_state, TCHAR_TO_ANSI(*type));
						lua_gettable(lua_state, -2);
						auto i = 0;
						while (lua_next(lua_state, -2) != 0) {
							xp::vchar render;
							lua_pushnumber(lua_state, i);
							lua_gettable(lua_state, -2);
							while (lua_next(lua_state, -2) != 0) {
								const FString tiletag = lua_tostring(lua_state, -2);
								if (tiletag == "glyph") render.glyph = static_cast<uint16_t>(lua_tonumber(lua_state, -1));
								if (tiletag == "foreground") render.foreground = read_lua_color("foreground");
								if (tiletag == "background") render.background = read_lua_color("background");
								lua_pop(lua_state, 1);
							}
							++i;
							c.glyphs.Emplace(render);
							lua_pop(lua_state, 1);
						}
						*/
					}
					lua_pop(lua_state, 1);
				}
			}
			if (field == "render_ascii") {
				lua_pushstring(lua_state, TCHAR_TO_ANSI(*field));
				lua_gettable(lua_state, -2);
				while (lua_next(lua_state, -2) != 0) {
					FString type = lua_tostring(lua_state, -2);
					if (type == "width") c.width = static_cast<int>(lua_tonumber(lua_state, -1));
					if (type == "height") c.height = static_cast<int>(lua_tonumber(lua_state, -1));
					if (type == "tiles") {
						lua_pushstring(lua_state, TCHAR_TO_ANSI(*type));
						lua_gettable(lua_state, -2);
						int i = 0;
						while (lua_next(lua_state, -2) != 0) {
							/*
							xp::vchar render;
							lua_pushnumber(lua_state, i);
							lua_gettable(lua_state, -2);
							while (lua_next(lua_state, -2) != 0) {
								const auto tiletag = FString(lua_tostring(lua_state, -2));
								if (tiletag == "glyph") render.glyph = static_cast<uint16_t>(lua_tonumber(lua_state, -1));
								if (tiletag == "foreground") render.foreground = read_lua_color("foreground");
								if (tiletag == "background") render.background = read_lua_color("background");
								lua_pop(lua_state, 1);
							}
							++i;
							c.glyphs_ascii.push_back(render);
							*/
							lua_pop(lua_state, 1);
						}
					}
					lua_pop(lua_state, 1);
				}
			}
			/*if (field == "render_rex") {
				const auto filename = buildings_path + FString(lua_tostring(lua_state, -1));
				xp::rex_sprite sprite(filename);
				c.width = sprite.get_width();
				c.height = sprite.get_height();
				for (auto y = 0; y<c.height; ++y) {
					for (auto x = 0; x<c.width; ++x) {
						const auto tmp = *sprite.get_tile(0, x, y);
						c.glyphs.push_back(tmp);
						c.glyphs_ascii.push_back(tmp);
					}
				}
				//std::cout << "Loaded REX file: " << filename << ", " << c.width << " x " << c.height << "\n";
			}*/

			lua_pop(lua_state, 1);
		}
		building_defs[c.hashtag] = c;
		//std::cout << "Read schematics for building: " << key << " (VOX " << building_defs[key].vox_model << ")\n";
		lua_pop(lua_state, 1);
	}
}

void NRaws::ReadReactions() {
	lua_getglobal(lua_state, "reactions");
	lua_pushnil(lua_state);

	while (lua_next(lua_state, -2) != 0)
	{
		reaction_t c;
		c.specials = TBitArray<FDefaultBitArrayAllocator>(false, 2);

		FString key = lua_tostring(lua_state, -2);
		c.tag = key;
		c.hashtag = GetTypeHash(c.tag);

		lua_pushstring(lua_state, TCHAR_TO_ANSI(*key));
		lua_gettable(lua_state, -2);
		while (lua_next(lua_state, -2) != 0) {
			FString field = lua_tostring(lua_state, -2);

			if (field == "name") c.name = lua_tostring(lua_state, -1);
			if (field == "emits_smoke") c.emits_smoke = lua_toboolean(lua_state, -1);
			if (field == "workshop") c.workshop = lua_tostring(lua_state, -1);
			if (field == "skill") c.skill = lua_tostring(lua_state, -1);
			if (field == "difficulty") c.difficulty = static_cast<int>(lua_tonumber(lua_state, -1));
			if (field == "inputs") {
				lua_pushstring(lua_state, TCHAR_TO_ANSI(*field));
				lua_gettable(lua_state, -2);
				while (lua_next(lua_state, -2) != 0) {
					lua_pushnil(lua_state);
					lua_gettable(lua_state, -2);

					reaction_input_t input;
					input.required_material = 0;
					input.required_material_type = NO_SPAWN_TYPE;
					while (lua_next(lua_state, -2) != 0) {
						const FString f = lua_tostring(lua_state, -2);
						if (f == "item") input.tag = lua_tostring(lua_state, -1);
						if (f == "qty") input.quantity = static_cast<int>(lua_tonumber(lua_state, -1));
						if (f == "material") {
							const FString mat_name = lua_tostring(lua_state, -1);
							const auto matfinder = get_material_by_tag(mat_name);
							if (matfinder == 0) {
								//glog(log_target::LOADER, log_severity::warning, "Reaction {0} references unkown material {1}", c.name, mat_name);
							}
							else {
								input.required_material = matfinder;
							}
						}
						if (f == "mat_type") {
							const FString type_s = lua_tostring(lua_state, -1);
							if (type_s == "cluster_rock") {
								input.required_material_type = material_def_spawn_type_t::CLUSTER_ROCK;
							}
							else if (type_s == "rock") {
								input.required_material_type = material_def_spawn_type_t::ROCK;
							}
							else if (type_s == "soil") {
								input.required_material_type = material_def_spawn_type_t::SOIL;
							}
							else if (type_s == "sand") {
								input.required_material_type = material_def_spawn_type_t::SAND;
							}
							else if (type_s == "metal") {
								input.required_material_type = material_def_spawn_type_t::METAL;
							}
							else if (type_s == "synthetic") {
								input.required_material_type = material_def_spawn_type_t::SYNTHETIC;
							}
							else if (type_s == "organic") {
								input.required_material_type = material_def_spawn_type_t::ORGANIC;
							}
							else if (type_s == "leather") {
								input.required_material_type = material_def_spawn_type_t::LEATHER;
							}
							else if (type_s == "food") {
								input.required_material_type = material_def_spawn_type_t::FOOD;
							}
							else if (type_s == "spice") {
								input.required_material_type = material_def_spawn_type_t::SPICE;
							}
							else {
								//glog(log_target::LOADER, log_severity::warning, "WARNING: Unknown material type: {0}", type_s);
							}
						}
						lua_pop(lua_state, 1);
					}
					c.inputs.Emplace(input);

					lua_pop(lua_state, 1);
				}
			}
			if (field == "outputs") {
				lua_pushstring(lua_state, TCHAR_TO_ANSI(*field));
				lua_gettable(lua_state, -2);
				while (lua_next(lua_state, -2) != 0) {
					lua_pushnil(lua_state);
					lua_gettable(lua_state, -2);
					TPair<FString, int> comp;
					while (lua_next(lua_state, -2) != 0) {
						const FString f = lua_tostring(lua_state, -2);
						if (f == "item") comp.Key = lua_tostring(lua_state, -1);
						if (f == "qty") comp.Value = static_cast<int>(lua_tonumber(lua_state, -1));
						if (f == "special") {
							const FString special = lua_tostring(lua_state, -1);
							if (special == "cooking") c.specials[special_reaction_cooking] = true;
							if (special == "tanning") c.specials[special_reaction_tanning] = true;
						}
						lua_pop(lua_state, 1);
					}
					c.outputs.Emplace(comp);
					//tech_tree_file << "\"" << key << "\"" << comp.first << "\"\n";

					lua_pop(lua_state, 1);
				}
			}
			if (field == "automatic") c.automatic = lua_toboolean(lua_state, -1);
			if (field == "power_drain") c.power_drain = static_cast<int>(lua_tonumber(lua_state, -1));

			lua_pop(lua_state, 1);
		}
		reaction_defs[c.hashtag] = c;
		reaction_building_defs[GetTypeHash(c.workshop)].Emplace(key);

		lua_pop(lua_state, 1);
	}
}

void NRaws::ReadPlantTypes() {
	plant_t p;
	FString tag;

	read_lua_table("vegetation",
		[&p, &tag](const auto &key) { tag = key; p = plant_t{}; p.tag = tag; },
		[&p,this](const auto &key) { plant_defs.Emplace(p); },
		lua_parser(
			std::initializer_list<lua_parser_inner_t>{
				{ "name", [&p, this]() { p.name = lua_str(); } },
				{ "cycles",  [&p,this]() {
					p.lifecycle.AddUninitialized(5);
					lua_pushstring(lua_state, "cycles");
					lua_gettable(lua_state, -2);
					while (lua_next(lua_state, -2) != 0)
					{
						const FString n = lua_tostring(lua_state, -2);
						int days = 0;
						days = lua_tonumber(lua_state, -1);

						if (n == "A") p.lifecycle[0] = days;
						if (n == "B") p.lifecycle[1] = days;
						if (n == "C") p.lifecycle[2] = days;
						if (n == "D") p.lifecycle[3] = days;
						if (n == "E") p.lifecycle[4] = days;
						lua_pop(lua_state, 1);
					}
				} },
				{ "glyphs_ascii",  [&p,this]() {
					lua_pushstring(lua_state, "glyphs_ascii");
					lua_gettable(lua_state, -2);
					//p.glyphs_ascii.resize(4);
					while (lua_next(lua_state, -2) != 0)
					{
						/*const FString n = lua_tostring(lua_state, -2);
						xp::vchar ap;
						lua_pushstring(lua_state, n.c_str());
						lua_gettable(lua_state, -2);
						while (lua_next(lua_state, -2) != 0)
						{
							const std::string field = lua_tostring(lua_state, -2);
							if (field == "glyph") ap.glyph = lua_tonumber(lua_state, -1);
							if (field == "col") ap.foreground = read_lua_color("col");
							lua_pop(lua_state, 1);
						}

						if (n == "A") p.glyphs_ascii[0] = ap;
						if (n == "B") p.glyphs_ascii[1] = ap;
						if (n == "C") p.glyphs_ascii[2] = ap;
						if (n == "D") p.glyphs_ascii[3] = ap;
						*/
						lua_pop(lua_state, 1);
					}
				} },
				{ "harvest", [&p,this]() {
					read_lua_table_inner("harvest", [&p](auto h) { p.provides.Emplace(h); });
				} },
				{ "tags",    [&p,this]() {
					read_lua_table_inner("tags", [&p](auto t) {
						//if (t == "spread") p.tags.set(PLANT_SPREADS);
						//if (t == "annual") p.tags.set(PLANT_ANNUAL);
					});
				} }
			}
		)
	);

	plant_defs.Sort([](plant_t a, plant_t b) { return a.tag < b.tag; });
	for (std::size_t i = 0; i<plant_defs.Num(); ++i) {
		plant_defs_idx[plant_defs[i].tag] = i;
	}
}

FNColor NRaws::ReadLuaColor(const FString &field) {
	FNColor col;
	lua_pushstring(lua_state, TCHAR_TO_ANSI(*field));
	lua_gettable(lua_state, -2);
	while (lua_next(lua_state, -2) != 0) {
		const FString subfield = lua_tostring(lua_state, -2);
		if (subfield == "r") col.r = static_cast<float>(lua_tonumber(lua_state, -1)) / 255.0f;
		if (subfield == "g") col.g = static_cast<float>(lua_tonumber(lua_state, -1)) / 255.0f;
		if (subfield == "b") col.b = static_cast<float>(lua_tonumber(lua_state, -1)) / 255.0f;
		lua_pop(lua_state, 1);
	}
	return col;
}

void NRaws::read_lua_table_inner(const FString &table, const TFunction<void(FString)> &functor)  noexcept
{
	lua_pushstring(lua_state, TCHAR_TO_ANSI(*table));
	lua_gettable(lua_state, -2);
	while (lua_next(lua_state, -2) != 0) {
		const FString s = lua_tostring(lua_state, -1);
		functor(s);
		lua_pop(lua_state, 1);
	}
}

void NRaws::read_lua_table_inner_p(const FString &table, const TFunction<void(FString)> &on_start, const TFunction<void(FString)> &on_end, const TMap<FString, const TFunction<void()>> &parser) noexcept
{
	lua_pushstring(lua_state, TCHAR_TO_ANSI(*table));
	lua_gettable(lua_state, -2);
	while (lua_next(lua_state, -2) != 0) {
		const FString key = lua_tostring(lua_state, -2);

		on_start(key);
		lua_pushstring(lua_state, TCHAR_TO_ANSI(*key));
		lua_gettable(lua_state, -2);
		while (lua_next(lua_state, -2) != 0) {
			const FString field = lua_tostring(lua_state, -2);
			call_functor_key(parser, field, table);
			lua_pop(lua_state, 1);
		}
		on_end(key);

		lua_pop(lua_state, 1);
	}
}

NRaws raws;