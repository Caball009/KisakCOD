local module = {}
local content = ""

local function content_append(data)
	content = content .. data .. "\n"
end

local function execute(command)
	local result, _ = os.outputof(command)

	return result
end

local function trim(str)
	return string.gsub(str, "^%s*(.-)%s*$", "%1")
end

local function get_host()
	local un = ""
	local cn = ""

	if os.istarget("windows") then
		local un_override = os.getenv("KISAK_BUILDNAME")
		local cn_override = os.getenv("KISAK_BUILDHOST")

		if un_override ~= nil then
			un = un_override
		else
			un = execute("echo %USERNAME%")
		end

		if cn_override ~= nil then
			cn = cn_override
		else
			cn = execute("echo %COMPUTERNAME%")
		end
	else
		un = execute("id -un")
		cn = execute("hostname")
	end

	return "\"" .. trim(un) .. "@" .. trim(cn) .. "\""
end

function module.update()
	local cwd = os.getcwd()
	local file = cwd .. "/tools/revision.txt"

	local revision_count = string.lines(execute("git rev-list HEAD"))
	local revision_hash = execute("git describe --always --dirty")
	local last_revision = io.readfile(file)
	
	if not last_revision then
		last_revision = "0"
	end

	if last_revision ~= revision_hash then
		print("Current revision is " .. revision_hash .. ".")
		print("Last revision is " .. last_revision .. ".")

		content_append("#pragma once\n")
		content_append("#define VERSION_NUMBER " .. revision_count)
		content_append("#define VERSION_NUMBER_STRING \"" .. revision_count .. "\"")
		content_append("#define VERSION_STRING \"" .. revision_hash .. "-" .. revision_count .. "\"")
		content_append("#define VERSION_HOST " .. get_host())

		io.writefile(cwd .. "/src/version.h", content)
		os.remove(file)
		io.writefile(file, revision_hash)

		print("Revision updated.")
	end
end

return module
