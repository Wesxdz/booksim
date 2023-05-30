function init(plugin)
  plugin:newCommand{
    id="insertBitmapText",
    title="Insert Bitmap Text",
    group="edit_menu",
    onclick=function()
      -- Load font sprite sheet.
      local fontSprite = Sprite { fromFile = plugin.path..'/fonts/ogre.png' }
      
      -- Character to sub-rectangle mapping.
      -- Each sub-rectangle is specified as {x, y, width, height}.
      -- These values should be adjusted according to your actual sprite sheet.
      local charMap = dofile(plugin.path..'/char-map.lua')

      -- Get user input.
      local text = app.prompt("Input Text", "Enter the text you want to render:", "")

      -- Calculate total width and height of the resulting image.
      local width, height = 0, 0
      for i = 1, #text do
        local char = text:sub(i, i)
        local charRect = charMap[char]

        width = width + charRect[3] -- Add the width of the character.
        if charRect[4] > height then
          height = charRect[4] -- Update the height if it's the tallest character so far.
        end
      end

      -- Create a new sprite to render the text.
      local textSprite = Sprite(width, height)

      -- Insert each character of the input text into the new sprite.
      local xOffset = 0
      for i = 1, #text do
        local char = text:sub(i, i)
        local charRect = charMap[char]

        -- Copy the character to the clipboard.
        app.command.CopyMerged { sprite = fontSprite, bounds = Rectangle(charRect[1], charRect[2], charRect[3], charRect[4]) }

        -- Paste the character into the new sprite.
        app.command.Paste { sprite = textSprite, x = xOffset, y = 0 }

        xOffset = xOffset + charRect[3] -- Move the x offset for the next character.
      end

      -- Display the new sprite.
      app.activeSprite = textSprite
    end
  }
end
