-- based on http://mk8.tockdom.com/wiki/MK8_Network_Protocol
local mk8base_proto = Proto("mk8base","Mk8Base")
local pf_magic = ProtoField.new("Magic", "mk8base.magic", ftypes.UINT32)
local pf_unknown0 = ProtoField.new("Unknown 0", "mk8base.unknown0", ftypes.UINT32)
local pf_timer = ProtoField.new("Timer", "mk8base.timer", ftypes.UINT16)
local pf_remote_timer = ProtoField.new("Remote Timer", "mk8base.remote_timer", ftypes.UINT16)
local pf_record = ProtoField.new("Record", "mk8base.record", ftypes.BYTES)
local pf_client_slot = ProtoField.new("Client slot", "mk8base.client_slot", ftypes.UINT16)
local pf_length = ProtoField.new("Length", "mk8base.length", ftypes.UINT16)
local pf_receiver_slots = ProtoField.new("Receiver slots", "mk8base.receiver_slots", ftypes.UINT32, nil, base.HEX)
local pf_sender_pid = ProtoField.new("Sender Pid", "mk8base.sender_pid", ftypes.UINT32)
local pf_unknown1 = ProtoField.new("Unknown 1", "mk8base.unknown1", ftypes.UINT32, nil, base.HEX)
local pf_unknown2 = ProtoField.new("Unknown 2", "mk8base.unknown2", ftypes.UINT32)
local pf_data = ProtoField.new("Data", "mk8base.data", ftypes.BYTES)

local pf_checksum = ProtoField.new("Checksum", "mk8base.checksum", ftypes.BYTES)

local pf_numrecords = ProtoField.new("Number of records", "mk8base.num_records", ftypes.UINT32)


mk8base_proto.fields = {
	pf_magic, pf_unknown0, pf_timer, pf_remote_timer,
	pf_record,
	pf_client_slot, pf_length, pf_receiver_slots, pf_sender_pid,
	pf_unknown1, pf_unknown2,
	pf_data,
	pf_checksum,
	pf_numrecords
}

-- dofile("C:\\Users\\zhuowei\\Documents\\winprogress\\mk8dissect\\mk8base.lua")

function mk8base_proto.dissector(tvbuf,pktinfo,root)
	local pktlen = tvbuf:reported_length_remaining()
	if pktlen < 4 then
		return 0
	end
	if tvbuf:range(0, 4):uint() ~= 0x32ab9864 then
		return 0
	end
	pktinfo.cols.protocol:set("MK8BASE")
	local tree = root:add(mk8base_proto, tvbuf:range(0,pktlen))
	tree:add(pf_magic, tvbuf:range(0,4))
	tree:add(pf_unknown0, tvbuf:range(4, 4))
	tree:add(pf_timer, tvbuf:range(8, 2))
	tree:add(pf_remote_timer, tvbuf:range(10, 2))
	local recordbegin = 12
	local numrecords = 0
	while recordbegin < (pktlen - 16) do
		numrecords = numrecords + 1
 		local datalen = tvbuf:range(recordbegin + 2, 2):uint()

		local recordlen = 20+datalen
		local recordmod = recordlen % 4
		if recordmod ~= 0 then
			recordlen = recordlen + (4-recordmod)
		end
		local stree = tree:add(pf_record, tvbuf:range(recordbegin, recordlen))
		stree:add(pf_client_slot, tvbuf:range(recordbegin + 0, 2))
		stree:add(pf_length, tvbuf:range(recordbegin + 2, 2))
		stree:add(pf_receiver_slots, tvbuf:range(recordbegin + 4, 4))
		stree:add(pf_sender_pid, tvbuf:range(recordbegin + 8, 4))
		stree:add(pf_unknown1, tvbuf:range(recordbegin + 12, 4))
		stree:add(pf_unknown2, tvbuf:range(recordbegin + 16, 4))
		stree:add(pf_data, tvbuf:range(recordbegin + 20, datalen))

		recordbegin = recordbegin + recordlen
	end
	tree:add(pf_checksum, tvbuf:range(pktlen - 16, 16))
	tree:add(pf_numrecords, numrecords)
	return pktlen
end

mk8base_proto:register_heuristic("udp", function (...) return mk8base_proto.dissector(...); end )