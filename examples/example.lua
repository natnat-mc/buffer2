local buffer2=require 'buffer2'

-- create a buffer to hold 16 shorts
local buf=buffer2.calloc(16, 'short')

-- fill it with powers of two
for i=1, #buf do
	buf[i]=2^(i-1)
end

-- read it as chars
buf.type='char'
for i=1, #buf do
	print(i, buf[i])
end
