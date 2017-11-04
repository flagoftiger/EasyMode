
-- UI elements
-- * IndicatorTexture

--
-- Global variables
--
local g_class = "None";
local g_spec = "None";

local g_onUpdateFunc = nil;
local g_onEventFunc = nil;
local g_initizlieFunc = nil;

local g_currentMove = 0;

-- Table <SpellName, <Key, SpellID> >
local g_spells = {};



--
-- Log functions
--
-- message:string
function LogDebug(message)
	DEFAULT_CHAT_FRAME:AddMessage("EasyMode: " .. (message or ""), 0.6, 0.6, 0.6);
end

function LogInfo(message)
	DEFAULT_CHAT_FRAME:AddMessage("EasyMode: " .. (message or ""));
end

function LogError(message)
	DEFAULT_CHAT_FRAME:AddMessage("EasyMode: " .. (message or "") , 1.0, 0.5, 0.5);
end

--
-- UI hander functions
--
function OnLoad(self)
	self:RegisterEvent("PLAYER_LOGIN")
end

function OnEvent(self, event, ...)
	if event == "PLAYER_LOGIN" then
		self:UnregisterEvent("PLAYER_LOGIN")
		self:RegisterEvent("UPDATE_BINDINGS")
		self:RegisterEvent("ACTIONBAR_UPDATE_COOLDOWN")
		self:RegisterEvent("AUTOFOLLOW_BEGIN")
		self:RegisterEvent("AUTOFOLLOW_END")
		self:RegisterEvent("PLAYER_TALENT_UPDATE")
		Initialize(self);
	elseif event == "UPDATE_BINDINGS" then
		-- Create spell table again		
	elseif event == "AUTOFOLLOW_BEGIN" then
		IndicatorTexture:SetVertexColor(tonumber("0x57") / tonumber("0xFF"), 0.0, 1.0, 1.0);


--		SendChatMessage("following", "PARTY", nil, nil)
	elseif event == "AUTOFOLLOW_END" then
		IndicatorTexture:SetVertexColor(tonumber("0x52") / tonumber("0xFF"), 1.0, 0.0, 1.0);

--		SendChatMessage("wait for me plz!!!", "PARTY", nil, nil)
	elseif event == "PLAYER_TALENT_UPDATE" then
		InitializeSpec();
	end

	-- Class & Spec specific OnEvent function
	if g_onEventFunc then
		_G[g_onEventFunc](self, event);
	end 
end

function OnUpdate(self, elapsed)
	if g_onUpdateFunc then
		_G[g_onUpdateFunc](elapsed);
	end
end

--
-- Initialization functions
--

function Initialize(self)
	LogInfo("EasyMode Enabled!");
	SetClass();
	InitializeSpec(self);

	--CreateSpellTable();
end

function InitializeSpec(self)
	SetSpec();
	g_onUpdateFunc = g_class .. "_" .. g_spec .. "_OnUpdate";
	if _G[g_onUpdateFunc] == nil then
		g_onUpdateFunc = nil;
	end
	g_onEventFunc = g_class .. "_" .. g_spec .. "_OnEvent";
	if _G[g_onEventFunc] == nil then
		g_onEventFunc = nil;
	end
	g_onInitializeFunc = g_class .. "_" .. g_spec .. "_Initialize";
	if _G[g_onInitializeFunc] then
		_G[g_onInitializeFunc](self);
	else
		g_onInitializeFunc = nil;
	end
	LogDebug("OnUpdate: " .. g_onUpdateFunc);
	LogDebug("OnEvent: " .. g_onEventFunc);
	LogDebug("Initialize: " .. g_onInitializeFunc);
end

function SetClass()
	g_class = select(2, UnitClass("PLAYER")) or "None";
end

function SetSpec()
	g_spec = select(2, GetSpecializationInfo(GetSpecialization())) or "None";
end

function CreateSpellTable()
end

--
-- Shared util functions
--

-- nextMove:byte
function SetNextMove(nextMove)
	if g_currentMove ~= nextMove then
		IndicatorTexture:SetVertexColor(nextMove / 255, 0.0, 0.0);
		g_currentMove = nextMove;
	end
end

-- return: 0.0~1.0
function GetPlayerPowerPercentage()
	return UnityPower("PLAYER") / UnityPowerMax("PLAYER");
end

-- return: 0.0~1.0
function GetPlayerHealthPercentage()
	return UnityHealth("PLAYER") / UnityHealthMax("PLAYER");
end

-- return:bool
function IsSpellReady(spellId)
	return GetSpellCooldown(spellId) == 0;
end

-- return:bool
function IsChannelling()
	a, b = UnitChannelInfo("PLAYER");
	return a ~= nil;
end

--
-- WARLOCK
--
-- Affliction
-- Demonology

--[[
"Shadow Bolt"
"Call Dreadstalkers"
"Hand of Gul'dan"
"Summon Doomguard"
"Summon Infernal"
"Grimoire: Felguard"
"Demonic Empowerment"
"Command Demon"
"Felstorm"
"Demonwrath"
"Thal'kiel's Consumption"
"Life Tap"

http://wowwiki.wikia.com/wiki/API_GetBinding

https://www.icy-veins.com/wow/demonology-warlock-pve-dps-rotation-cooldowns-abilities

]]--
function WARLOCK_Demonology_Initialize(self)
	self:RegisterEvent("COMBAT_LOG_EVENT_UNFILTERED")
end

function WARLOCK_Demonology_OnEvent(self, event)
	if event == "COMBAT_LOG_EVENT_UNFILTERED" then
	end
end

function WARLOCK_Demonology_Spell()
	g_spells = {
		["Shadow Bolt"] = {"2"},
		["Call Dreadstalkers"] = {"T"},
		["Hand of Gul'dan"] = {"Q"},
		["Summon Doomguard"] = {"F1"},
		["Summon Infernal"] = {"F3"},
		["Grimoire: Felguard"] = {"3"},
		["Demonic Empowerment"] = {"R"},
		["Command Demon"] = {""},
		["Felstorm"] = {},
		["Demonwrath"] = {"C"},
		["Thal'kiel's Consumption"] = {"V"},
		["Life Tap"] = {"CapsLock"},
	}

	for spellName in pairs(g_spells) do 

	end	
end

function WARLOCK_Demonology_OnUpdate(elapsed)
	name, a  = UnitBuff("Dreadstalker", 1);
	if name then
		LogDebug(name);
	end


--	LogDebug("WARLOCK_Demonology run");
--[[
	-- check casting something or not
	if casting then
		return nil;
	end

	if GetMP() < 20% then
		return Spell["Life Tap"];

	-- check having any demons




	-- check souldShard count
	soulShardCount = GetSoulShardCount();
	if (not Doomgaurd or not Infernal)
		summon Doomgard or Infernal

	-- if soulShard count > 2
	--	Call Dreadstalkers
]]--
end
-- Destruction

