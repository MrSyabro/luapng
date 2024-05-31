---@meta

---@class PNGUtils
local M = {}

---Read data from file and convert HWC to CHW
---@param inputfile string
---@return {height: integer, width: integer}|number[] data
function M.read(inputfile)
end

---Read grayscale data from file and convert HWC to CHW
---@param inputfile string
---@return {height: integer, width: integer}|number[] data
function M.read_grayscale(inputfile)
end

---Convert CHW to HWC and write data to file
---@param data {height: integer, width: integer}|number[]
---@param outputfile string имя файла
function M.write(data, outputfile)
end

return M