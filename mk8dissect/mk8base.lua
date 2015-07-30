-- based on http://mk8.tockdom.com/wiki/MK8_Network_Protocol
local mk8base_proto = Proto("mk8base","Mk8Base")
local pf_magic = ProtoField.new("Magic", "mk8base.magic", ftypes.UINT32)
local pf_unknown0 = ProtoField.new("Unknown 0", "mk8base.unknown0", ftypes.UINT32)
local pf_timer = ProtoField.new("Timer", "mk8base.timer", ftypes.UINT16)
local pf_remote_timer = ProtoField.new("Remote Timer", "mk8base.remote_timer", ftypes.UINT16)
local pf_client_slot = ProtoField.new("Client slot", "mk8base.client_slot", ftypes.UINT16)
local pf_length = ProtoField.new("Length", "mk8base.length", ftypes.UINT16)
local pf_receiver_slots = ProtoField.new("Receiver slots", "mk8base.receiver_slots", ftypes.UINT32, nil, base.HEX)
local pf_sender_pid = ProtoField.new("Sender Pid", "mk8base.sender_pid", ftypes.UINT32)
local pf_unknown1 = ProtoField.new("Unknown 1", "mk8base.unknown1", ftypes.UINT32, nil, base.HEX)
local pf_unknown2 = ProtoField.new("Unknown 2", "mk8base.unknown2", ftypes.UINT32)
local pf_data = ProtoField.new("Data", "mk8base.data", ftypes.BYTES)

local pf_checksum = ProtoField.new("Checksum", "mk8base.checksum", ftypes.BYTES)

mk8base_proto.fields = {
	pf_magic, pf_unknown0, pf_timer, pf_remote_timer,
	pf_client_slot, pf_length, pf_receiver_slots, pf_sender_pid,
	pf_unknown1, pf_unknown2,
	pf_data,
	pf_checksum
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
	tree:add(pf_client_slot, tvbuf:range(12, 2))
	tree:add(pf_length, tvbuf:range(14, 2))
	tree:add(pf_receiver_slots, tvbuf:range(16, 4))
	tree:add(pf_sender_pid, tvbuf:range(20, 4))
	tree:add(pf_unknown1, tvbuf:range(24, 4))
	tree:add(pf_unknown2, tvbuf:range(28, 4))
	tree:add(pf_data, tvbuf:range(32, tvbuf:range(14, 2):uint()))

	tree:add(pf_checksum, tvbuf:range(pktlen - 16, 16))
	return pktlen
end

mk8base_proto:register_heuristic("udp", function (...) return mk8base_proto.dissector(...); end )