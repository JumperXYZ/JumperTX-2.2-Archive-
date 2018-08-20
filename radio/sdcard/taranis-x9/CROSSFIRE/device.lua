---- #########################################################################
---- #                                                                       #
---- # Copyright (C) OpenTX                                                  #
-----#                                                                       #
---- # License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html               #
---- #                                                                       #
---- # This program is free software; you can redistribute it and/or modify  #
---- # it under the terms of the GNU General Public License version 2 as     #
---- # published by the Free Software Foundation.                            #
---- #                                                                       #
---- # This program is distributed in the hope that it will be useful        #
---- # but WITHOUT ANY WARRANTY; without even the implied warranty of        #
---- # MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
---- # GNU General Public License for more details.                          #
---- #                                                                       #
---- #########################################################################
local deviceId = 0
local deviceName = ""
local lineIndex = 0
local pageOffset = 0
local edit = false
local charIndex = 1
local fieldPopup
local fieldTimeout = 0
local fieldId = 1
local fieldChunk = 0
local fieldData = {}
local fields = {}

local function getField(line)
  local counter = 1
  for i = 1, #fields do
    local field = fields[i]
    if not field.hidden then
      if counter < line then
        counter = counter + 1
      else
        return field
      end
    end
  end
end

local function initLineIndex()
  lineIndex = 0
  for i = 1, #fields do
    local field = getField(i)
    if field and field.type ~= 11 and field.type ~= 12 and field.name ~= nil then
      lineIndex = i
      break
    end
  end
end

-- Change display attribute to current field
local function incrField(step)
  local field = getField(lineIndex)
  if field.type == 10 then
    local byte = 32
    if charIndex <= #field.value then
      byte = string.byte(field.value, charIndex) + step
    end
    if byte < 32 then
      byte = 32
    elseif byte > 122 then
      byte = 122
    end
    if charIndex <= #field.value then
      field.value = string.sub(field.value, 1, charIndex-1) .. string.char(byte) .. string.sub(field.value, charIndex+1)
    else
      field.value = field.value .. string.char(byte)
    end
  else
    local min, max = 0, 0
    if field.type <= 5 then
      min = field.min
      max = field.max
      step = field.step * step
    elseif field.type == 9 then
      min = 0
      max = #field.values - 1
    end
    if (step < 0 and field.value > min) or (step > 0 and field.value < max) then
      field.value = field.value + step
    end
  end
end

-- Select the next or previous editable field
local function selectField(step)
  local newLineIndex = lineIndex
  local field
  repeat
    newLineIndex = newLineIndex + step
    if newLineIndex == 0 then
      newLineIndex = #fields
    elseif newLineIndex == 1 + #fields then
      newLineIndex = 1
      pageOffset = 0
    end
    field = getField(newLineIndex)
  until newLineIndex == lineIndex or (field and field.type ~= 11 and field.name)
  lineIndex = newLineIndex
  if lineIndex > 7 + pageOffset then
    pageOffset = lineIndex - 7
  elseif lineIndex <= pageOffset then
    pageOffset = lineIndex - 1
  end
end

local function split(str)
  local t = {}
  local i = 1
  for s in string.gmatch(str, "([^;]+)") do
    t[i] = s
    i = i + 1
  end
  return t
end

local function fieldGetString(data, offset)
  local result = ""
  while data[offset] ~= 0 do
    result = result .. string.char(data[offset])
    offset = offset + 1
  end
  offset = offset + 1
  return result, offset
end

local function parseDeviceInfoMessage(data)
  local offset
  deviceId = data[2]
  deviceName, offset = fieldGetString(data, 3)
  local fields_count = data[offset+12]
  for i=1, fields_count do
    fields[i] = { name=nil }
  end
end

local function fieldGetValue(data, offset, size)
  local result = 0
  for i=0, size-1 do
    result = bit32.lshift(result, 8) + data[offset + i]
  end
  return result
end

local function fieldUnsignedLoad(field, data, offset, size)
  field.value = fieldGetValue(data, offset, size)
  field.min = fieldGetValue(data, offset+size, size)
  field.max = fieldGetValue(data, offset+2*size, size)
  field.default = fieldGetValue(data, offset+3*size, size)
  field.unit, offset = fieldGetString(data, offset+4*size)
  field.step = 1
end

local function fieldUnsignedToSigned(field, size)
  local bandval = bit32.lshift(0x80, (size-1)*8)
  field.value = field.value - bit32.band(field.value, bandval) * 2
  field.min = field.min - bit32.band(field.min, bandval) * 2
  field.max = field.max - bit32.band(field.max, bandval) * 2
  field.default = field.default - bit32.band(field.default, bandval) * 2
end

  local function fieldSignedLoad(field, data, offset, size)
  fieldUnsignedLoad(field, data, offset, size)
  fieldUnsignedToSigned(field, size)
end

local function fieldIntSave(index, value, size)
  local frame = { deviceId, 0xEA, index }
  for i=size-1, 0, -1 do
    frame[#frame + 1] = (bit32.rshift(value, 8*i) % 256)
  end
  crossfireTelemetryPush(0x2D, frame)
end

local function fieldUnsignedSave(field, size)
  local value = field.value
  fieldIntSave(field.id, value, size)
end

local function fieldSignedSave(field, size)
  local value = field.value
  if value < 0 then
    value = bit32.lshift(0x100, (size-1)*8) + value
  end
  fieldIntSave(field.id, value, size)
end

local function fieldIntDisplay(field, y, attr)
  lcd.drawNumber(140, y, field.value, LEFT + attr)
  lcd.drawText(lcd.getLastPos(), y, field.unit, attr)
end

-- UINT8
local function fieldUint8Load(field, data, offset)
  fieldUnsignedLoad(field, data, offset, 1)
end

local function fieldUint8Save(field)
  fieldUnsignedSave(field, 1)
end

-- INT8
local function fieldInt8Load(field, data, offset)
  fieldSignedLoad(field, data, offset, 1)
end

local function fieldInt8Save(field)
  fieldSignedSave(field, 1)
end

-- UINT16
local function fieldUint16Load(field, data, offset)
  fieldUnsignedLoad(field, data, offset, 2)
end

local function fieldUint16Save(field)
  fieldUnsignedSave(field, 2)
end

-- INT16
local function fieldInt16Load(field, data, offset)
  fieldSignedLoad(field, data, offset, 2)
end

local function fieldInt16Save(field)
  fieldSignedSave(field, 2)
end

-- FLOAT
local function fieldFloatLoad(field, data, offset)
  field.value = fieldGetValue(data, offset, 4)
  field.min = fieldGetValue(data, offset+4, 4)
  field.max = fieldGetValue(data, offset+8, 4)
  field.default = fieldGetValue(data, offset+12, 4)
  fieldUnsignedToSigned(field, 4)
  field.prec = data[offset+16]
  if field.prec > 2 then
    field.prec = 2
  end
  field.step = fieldGetValue(data, offset+17, 4)
  field.unit, offset = fieldGetString(data, offset+21)
end

local function fieldFloatDisplay(field, y, attr)
  local attrnum
  if field.prec == 1 then
    attrnum = LEFT + attr + PREC1
  elseif field.prec == 2 then
    attrnum = LEFT + attr + PREC2
  else
    attrnum = LEFT + attr
  end
  lcd.drawNumber(140, y, field.value, attrnum)
  lcd.drawText(lcd.getLastPos(), y, field.unit, attr)
end

local function fieldFloatSave(field)
  fieldUnsignedSave(field, 4)
end

-- TEXT SELECTION
local function fieldTextSelectionLoad(field, data, offset)
  local values
  values, offset = fieldGetString(data, offset)
  if values ~= "" then
    field.values = split(values)
  end
  field.value = data[offset]
  field.min = data[offset+1]
  field.max = data[offset+2]
  field.default = data[offset+3]
  field.unit, offset = fieldGetString(data, offset+4)
end

local function fieldTextSelectionSave(field)
  crossfireTelemetryPush(0x2D, { deviceId, 0xEA, field.id, field.value })
end

local function fieldTextSelectionDisplay(field, y, attr)
  lcd.drawText(140, y, field.values[field.value+1], attr)
  lcd.drawText(lcd.getLastPos(), y, field.unit, attr)
end

-- STRING
local function fieldStringLoad(field, data, offset)
  field.value, offset = fieldGetString(data, offset)
  if #data >= offset then
    field.maxlen = data[offset]
  end
end

local function fieldStringSave(field)
  local frame = { deviceId, 0xEA, field.id }
  for i=1, string.len(field.value) do
    frame[#frame + 1] = string.byte(field.value, i)
  end
  frame[#frame + 1] = 0
  crossfireTelemetryPush(0x2D, frame)
end

local function fieldStringDisplay(field, y, attr)
  if edit == true and attr then
    lcd.drawText(140, y, field.value, FIXEDWIDTH)
    lcd.drawText(134+6*charIndex, y, string.sub(field.value, charIndex, charIndex), FIXEDWIDTH + attr)
  else
    lcd.drawText(140, y, field.value, attr)
  end
end

local function fieldCommandLoad(field, data, offset)
  field.status = data[offset]
  field.timeout = data[offset+1]
  field.info, offset = fieldGetString(data, offset+2)
  if field.status < 2 or field.status > 3 then
    fieldPopup = nil
  end
end

local function fieldCommandSave(field)
  if field.status == 0 then
    field.status = 1
    crossfireTelemetryPush(0x2D, { deviceId, 0xEA, field.id, field.status })
    fieldPopup = field
    fieldTimeout = getTime() + field.timeout
  end
end

local function fieldCommandDisplay(field, y, attr)
  lcd.drawText(0, y, field.name, attr)
  if field.info ~= "" then
    lcd.drawText(140, y, "[" .. field.info .. "]")
  end
end

local functions = {
  { load=fieldUint8Load, save=fieldUint8Save, display=fieldIntDisplay },
  { load=fieldInt8Load, save=fieldInt8Save, display=fieldIntDisplay },
  { load=fieldUint16Load, save=fieldUint16Save, display=fieldIntDisplay },
  { load=fieldInt16Load, save=fieldInt16Save, display=fieldIntDisplay },
  nil,
  nil,
  nil,
  nil,
  { load=fieldFloatLoad, save=fieldFloatSave, display=fieldFloatDisplay },
  { load=fieldTextSelectionLoad, save=fieldTextSelectionSave, display=fieldTextSelectionDisplay },
  { load=fieldStringLoad, save=fieldStringSave, display=fieldStringDisplay },
  nil,
  { load=fieldStringLoad, save=fieldStringSave, display=fieldStringDisplay },
  { load=fieldCommandLoad, save=fieldCommandSave, display=fieldCommandDisplay },
}

local function parseParameterInfoMessage(data)
  if data[2] ~= deviceId or data[3] ~= fieldId then
    fieldData = {}
    fieldChunk = 0
    return
  end
  local field = fields[fieldId]
  local chunks = data[4]
  for i=5, #data do
    fieldData[#fieldData + 1] = data[i]
  end
  if chunks > 0 then
    fieldChunk = fieldChunk + 1
  else
    fieldChunk = 0
    field.id = fieldId
    field.parent = fieldData[1]
    field.type = fieldData[2] % 128
    field.hidden = (bit32.rshift(fieldData[2], 7) == 1)
    local name, i = fieldGetString(fieldData, 3)
    if name ~= "" then
      local indent = ""
      local parent = field.parent
      while parent ~= 0 do
        indent = indent .. " "
        parent = fields[parent].parent
      end
      field.name = indent .. name
    end
    if functions[field.type+1] then
      functions[field.type+1].load(field, fieldData, i)
    end
    if not fieldPopup then
      if lineIndex == 0 and field.hidden ~= true and field.type and field.type ~= 11 and field.type ~= 12 then
        initLineIndex()
      end
      fieldId = 1 + (fieldId % #fields)
    end
    fieldData = {}
  end
end

local function refreshNext()
  local command, data = crossfireTelemetryPop()
  if command == nil then
    local time = getTime()
    if fieldPopup then
      if time > fieldTimeout then
        local frame = { deviceId, 0xEA, fieldPopup.id }
        crossfireTelemetryPush(0x2D, frame)
        fieldTimeout = time + fieldPopup.timeout
      end
    elseif time > fieldTimeout and not edit then
      crossfireTelemetryPush(0x2C, { deviceId, 0xEA, fieldId, fieldChunk })
      fieldTimeout = time + 200 -- 2s
    end
  elseif command == 0x29 then
    parseDeviceInfoMessage(data)
  elseif command == 0x2B then
    parseParameterInfoMessage(data)
    fieldTimeout = 0
  end
end

-- Main
local function runDevicePage(event)
  if event == EVT_EXIT_BREAK then             -- exit script
    if edit == true then
      edit = false
      local field = getField(lineIndex)
      fieldTimeout = getTime() + 200 -- 2s
      fieldId, fieldChunk = field.id, 0
      fieldData = {}
      functions[field.type+1].save(field)
    else
      return "crossfire.lua"
    end
  elseif event == EVT_ENTER_BREAK then        -- toggle editing/selecting current field
    local field = getField(lineIndex)
    if field.name then
      if field.type == 10 then
        if edit == false then
          edit = true
          charIndex = 1
        else
          charIndex = charIndex + 1
        end
      elseif field.type < 11 then
        edit = not edit
      end
      if edit == false then
        fieldTimeout = getTime() + 200 -- 2s
        fieldId, fieldChunk = field.id, 0
        fieldData = {}
        functions[field.type+1].save(field)
      end
    end
  elseif edit then
    if event == EVT_PLUS_FIRST or event == EVT_PLUS_REPT or event == EVT_ROT_LEFT then
      incrField(1)
    elseif event == EVT_MINUS_FIRST or event == EVT_MINUS_REPT or event == EVT_ROT_RIGHT then
      incrField(-1)
    end
  else
    if event == EVT_MINUS_FIRST or event == EVT_ROT_RIGHT then
      selectField(1)
    elseif event == EVT_PLUS_FIRST or event == EVT_ROT_LEFT then
      selectField(-1)
    end
  end

  lcd.clear()
  lcd.drawScreenTitle(deviceName, 0, 0)
  for y = 1, 7 do
    local field = getField(pageOffset+y)
    if not field then
      break
    elseif field.name == nil then
      lcd.drawText(0, 1+8*y, "...")
    else
      local attr = lineIndex == (pageOffset+y) and ((edit == true and BLINK or 0) + INVERS) or 0
      lcd.drawText(0, 1+8*y, field.name)
      if functions[field.type+1] then
        functions[field.type+1].display(field, 1+8*y, attr)
      end
    end
  end

  return 0
end

local function runPopupPage(event)
  local result
  if fieldPopup.status == 3 then
    result = popupConfirmation(fieldPopup.info, event)
  else
    result = popupWarning(fieldPopup.info, event)
  end
  if result == "OK" then
    crossfireTelemetryPush(0x2D, { deviceId, 0xEA, fieldPopup.id, 4 })
  elseif result == "CANCEL" then
    crossfireTelemetryPush(0x2D, { deviceId, 0xEA, fieldPopup.id, 5 })
  end
  return 0
end

-- Init
local function init()
  lineIndex, edit = 0, false
end

-- Main
local function run(event)
  if event == nil then
    error("Cannot be run as a model script!")
    return 2
  end

  local result
  if fieldPopup ~= nil then
    result = runPopupPage(event)
  else
    result = runDevicePage(event)
  end

  refreshNext()

  return result
end

return { init=init, run=run }
