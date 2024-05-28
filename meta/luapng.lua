---@meta

---@class PNGUtils
local M = {}

---Read data from file and convert HWC to CHW
---@param inputfile any
---@return string data
---@return number height
---@return number width
function M.read(inputfile)
end

---Read grayscale data from file and convert HWC to CHW
---@param inputfile any
---@return string data
---@return number height
---@return number width
function M.read_grayscale(inputfile)
end

---Convert CHW to HWC and write data to file
---@param data string данные для записи
---@param height integer
---@param width integer
---@param outputfile string имя файла
function M.write(data, height, width, outputfile)
end

return M