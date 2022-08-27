
// c sTyLe cAsTiNg

const auto rbxscriptsignal_scriptconnection = 0x1C;
const auto rbxscriptsignal_nextsignal = 0x10;
const auto rbxscriptsignal_state = 0x14;



namespace connection_environment
{
	struct connections
	{
		std::uintptr_t Object;
		std::uintptr_t OldState;
	};

	std::unordered_map<std::uintptr_t, connections> Connections;

	std::int32_t enable_connection(std::uintptr_t rL)
	{
		const auto Signal = *(DWORD*)r_lua_touserdata(rL, 1);//*reinterpret_cast<std::uintptr_t*>(r_lua_touserdata(rL, 1));

		if (!Connections.count(Signal))
		{
			connections New;
			New.Object = Signal;
			New.OldState = *reinterpret_cast<std::uintptr_t*>(Signal + rbxscriptsignal_state);

			Connections[Signal] = New;
		}

		*reinterpret_cast<std::uintptr_t*>(Signal + rbxscriptsignal_state) = Connections[Signal].OldState;
		return 0;
	}

	std::int32_t disable_connection(std::uintptr_t rL)
	{
		const auto Signal = *reinterpret_cast<std::uintptr_t*>(r_lua_touserdata(rL, 1));

		if (!Connections.count(Signal))
		{
			connections New;
			New.Object = Signal;
			New.OldState = *reinterpret_cast<std::uintptr_t*>(Signal + rbxscriptsignal_state);

			Connections[Signal] = New;
		}

		*reinterpret_cast<std::uintptr_t*>(Signal + rbxscriptsignal_state) = 0;
		return 0;
	}


	std::int32_t blank_function(std::uintptr_t rL)
	{
		return 0;
	}

	std::int32_t index_connection(std::uintptr_t rL)
	{
		const std::string Key = r_lua_tolstring(rL, 2, nullptr);

		if (Key == "Enable" || Key == "enable")
		{
			r_lua_pushvalue(rL, 1);
			r_lua_pushcfunction(rL, enable_connection, 0);
		}
		else if (Key == "Function" || Key == "function" || Key == "Fire" || Key == "fire")
		{
			const auto connection = *reinterpret_cast<connections*>(r_lua_touserdata(rL, 1));
			r_lua_pushnumber(rL, *reinterpret_cast<std::uintptr_t*>(*reinterpret_cast<std::uintptr_t*>(*reinterpret_cast<std::uintptr_t*>(connection.Object + rbxscriptsignal_scriptconnection) + 0x64) + 0xC));
			r_lua_rawget(rL, R_LUA_REGISTRYINDEX);

			if (r_lua_type(rL, -1) <= 0)
				r_lua_pushcfunction(rL, blank_function, 0);
		}
		else if (Key == "Enabled" || Key == "enabled")
		{
			const auto Signal = *reinterpret_cast<std::uintptr_t*>(r_lua_touserdata(rL, 1));

			const auto conn = *reinterpret_cast<std::uintptr_t*>(Signal + rbxscriptsignal_state);

			r_lua_pushboolean(rL, conn);
			//*reinterpret_cast<std::uintptr_t*>(Signal + rbxscriptsignal_state) = 0;
		}
		else
		{
			r_lua_pushvalue(rL, 1);
			r_lua_pushcfunction(rL, disable_connection, 0);
		}

		return 1;
	}
}


std::int32_t environment::get_connections(std::uintptr_t rL)
{
	//const auto R_LUA_TUSERDATA = r_get_offset(r_offsets::R_LUAT_USERDATA);

	r_luaL_checktype(rL, 1, R_LUA_TUSERDATA);

	

	r_lua_getfield(rL, 1, "connect");
	r_lua_pushvalue(rL, 1);
	r_lua_pushcfunction(rL, connection_environment::blank_function, 0);
	r_lua_pcall(rL, 2, 1, 0);

	const auto signal = *reinterpret_cast<std::uintptr_t*>(r_lua_touserdata(rL, -1));
	auto next = *reinterpret_cast<std::uintptr_t*>(signal + rbxscriptsignal_nextsignal);

	//r_lua_createtable(rL, 0, 0);
	r_lua_createtable(rL, 0, 0);
	auto Count = 1;

	while (next != 0)
	{
		
		if (connection_environment::Connections.count(next))
		{
			*reinterpret_cast<connection_environment::connections*>(r_lua_newuserdata(rL, sizeof(connection_environment::connections), 0)) = connection_environment::Connections[next];

			//r_lua_createtable(rL, 0, 0);
			r_lua_createtable(rL, 0, 0);
			r_lua_pushcfunction(rL, connection_environment::index_connection, 0);
			r_lua_setfield(rL, -2, "__index");
			r_lua_pushstring(rL, "table");
			r_lua_setfield(rL, -2, "__type");
			r_lua_setmetatable(rL, -2);
		}
		else
		{
			connection_environment::connections new_connection;
			new_connection.Object = next;
			new_connection.OldState = *reinterpret_cast<std::uintptr_t*>(next + rbxscriptsignal_state);

			*reinterpret_cast<connection_environment::connections*>(r_lua_newuserdata(rL, sizeof(connection_environment::connections), 0)) = new_connection;

			r_lua_createtable(rL, 0, 0);
			//r_lua_newtable(rL);
			r_lua_pushcfunction(rL, connection_environment::index_connection, 0);
			r_lua_setfield(rL, -2, "__index");
			r_lua_pushstring(rL, "table");
			r_lua_setfield(rL, -2, "__type");
			r_lua_setmetatable(rL, -2);

			connection_environment::Connections[next] = new_connection;
		}

		r_lua_rawseti(rL, -2, Count++);
		next = *reinterpret_cast<std::uintptr_t*>(next + rbxscriptsignal_nextsignal);
	}

	r_lua_getfield(rL, -2, "Disconnect");
	r_lua_pushvalue(rL, -3);
	r_lua_pcall(rL, 1, 0, 0);

	return 1;
}
