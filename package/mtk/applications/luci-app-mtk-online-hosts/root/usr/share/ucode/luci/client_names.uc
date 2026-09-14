// SPDX-License-Identifier: Apache-2.0
// RFC 1002 NBSTAT: unicast queries only, to known LAN clients.
import { create, poll, AF_INET, SOCK_DGRAM, POLLIN, MSG_DONTWAIT } from 'socket';

function byte(data, offset) { return ord(substr(data, offset, 1)); };
function word(data, offset) { return byte(data, offset) * 256 + byte(data, offset + 1); };
function skip_name(data, offset) {
	while (offset < length(data)) {
		let size = byte(data, offset++);
		if (!size) return offset;
		if ((size & 0xc0) == 0xc0)
			return offset < length(data) ? offset + 1 : -1;
		if (size > 63 || offset + size > length(data)) return -1;
		offset += size;
	}
	return -1;
};

export function nbstat_name(data, id) {
	if (type(data) != 'string' || length(data) < 12 || word(data, 0) != id ||
	    (word(data, 2) & 0xfa0f) != 0x8000 || word(data, 6) != 1)
		return null;
	let offset = 12;
	for (let i = 0; i < word(data, 4); i++) {
		offset = skip_name(data, offset);
		if (offset < 0 || offset + 4 > length(data)) return null;
		offset += 4;
	}
	offset = skip_name(data, offset);
	if (offset < 0 || offset + 11 > length(data) || word(data, offset) != 0x21 ||
	    word(data, offset + 2) != 1) return null;
	let size = word(data, offset + 8);
	offset += 10;
	if (size < 1 || offset + size > length(data)) return null;
	let count = byte(data, offset++), server = null;
	if (1 + count * 18 > size) return null;
	for (let i = 0; i < count; i++, offset += 18) {
		let suffix = byte(data, offset + 15), flags = word(data, offset + 16);
		let name = trim(substr(data, offset, 15));
		if ((flags & 0x8000) || !match(name, /^[a-z0-9_][a-z0-9_.-]*$/i)) continue;
		if (suffix == 0) return name;
		if (suffix == 0x20) server = name;
	}
	return server;
};

export function netbios_names(addresses) {
	let jobs = [], result = {};
	for (let ip in addresses) {
		if (length(jobs) >= 32) break;
		let sock = create(AF_INET, SOCK_DGRAM);
		if (!sock) continue;
		let id = (int(time()) + length(jobs)) & 65535;
		let query = chr(id >> 8) + chr(id & 255) + '\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00' +
			'\x20CKAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\x00\x00\x21\x00\x01';
		if (!sock.connect({ family: AF_INET, address: ip, port: 137 }) || sock.send(query) != length(query)) {
			sock.close();
			continue;
		}
		push(jobs, [sock, POLLIN, ip, id]);
	}
	// Batch all clients; never wait one second per address.
	for (let round = 0; round < 4 && length(jobs); round++) {
		let events = poll(250, ...jobs) || [];
		for (let event in events) {
			if (!event[1]) continue;
			let name = nbstat_name(event[0].recv(4096, MSG_DONTWAIT), event[3]);
			if (name) result[event[2]] = name;
			event[0].close();
			jobs = filter(jobs, job => job[2] != event[2]);
		}
	}
	for (let job in jobs) job[0].close();
	return result;
};
