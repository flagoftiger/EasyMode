
-- UI elements
-- * IndicatorTexture

--
-- Global variables
--
local g_class = "None";
local g_spec = "None";

local g_moveFunc = "None";

function Log(message)
	DEFAULT_CHAT_FRAME:AddMessage("D " .. (message or ""));
end

function SetNextKey()
	IndicatorTexture:SetVertexColor(0.0, 0.0, 0.0, 1.0);
end

--
-- AddOn entry point
--
function OnLoad(self)
	Log("OnLoad");
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
		Initialize();
	elseif event == "UPDATE_BINDINGS" then
		-- Create spell table again		
	elseif event == "AUTOFOLLOW_BEGIN" then
		IndicatorTexture:SetVertexColor(tonumber("0x57") / tonumber("0xFF"), 0.0, 1.0, 1.0);


		SendChatMessage("following", "PARTY", nil, nil)
	elseif event == "AUTOFOLLOW_END" then
		IndicatorTexture:SetVertexColor(tonumber("0x52") / tonumber("0xFF"), 1.0, 0.0, 1.0);

		SendChatMessage("wait for me plz!!!", "PARTY", nil, nil)
	elseif event == "PLAYER_TALENT_UPDATE" then
		SetSpec();
		g_moveFunc = g_class .. "_" .. g_spec;
		Log("PLAYER_TALENT_UPDATE: " .. g_moveFunc);
	end
end

function Initialize()
	Log("EasyMode loaded!");
	SetClass();
	SetSpec();

	g_moveFunc = g_class .. "_" .. g_spec;

	Log(g_moveFunc);
	_G[g_moveFunc]();

	--CreateSpellTable();
	--GetNextMove();
end

function SetClass()
	g_class = select(2, UnitClass("PLAYER")) or "None";
end

function SetSpec()
	g_spec = select(2, GetSpecializationInfo(GetSpecialization())) or "None";
end

function CreateSpellTable()
end

function GetNextMove()
	local controllerName = "GetNextMove_" .. g_class .. "_" .. g_spec;
	local key, spell = _G[controllerName]();
end

function GetNextMove_SHAMAN_Restoration()
	Log("GetNextMove_SHAMAN_Restoration");
	return nil, nil;
end
function WARLOCK_Demonology()
	Log("WARLOCK_Demonology run");
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
