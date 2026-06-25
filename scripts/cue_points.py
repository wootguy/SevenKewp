import sys, os, wave, struct

def get_wave_cue_points(fpath):	
	sample_rate = 0
	bytes_per_sample = 0
	read = 0
	file = open(fpath, "rb")

	file.seek(0, os.SEEK_END)
	fsize = file.tell()
	file.seek(0, os.SEEK_SET)

	# Read RIFF_HEADER
	riff_str = file.read(4)
	overall_size = int.from_bytes(file.read(4), byteorder='little')
	wave_str = file.read(4)

	if riff_str != b'RIFF' or wave_str != b'WAVE':
		print("Invalid WAVE header: {}".format(fpath))
		return []

	# Read other chunks
	while True:
		if (file.tell() + 8 >= fsize):
			break
			
		chunk_name = file.read(4).decode('ascii', 'ignore')
		chunk_size = int.from_bytes(file.read(4), byteorder='little')
		padded_size = int((chunk_size + 1) / 2) * 2

		if chunk_name == "cue ":
			numCues = int.from_bytes(file.read(4), byteorder='little')
			
			cues = []
			
			for x in range(0, numCues):
				id = int.from_bytes(file.read(4), byteorder='little')
				position = int.from_bytes(file.read(4), byteorder='little')
				chunkId = file.read(4).decode('utf-8')
				chunkStart = int.from_bytes(file.read(4), byteorder='little')
				blockStart = int.from_bytes(file.read(4), byteorder='little')
				sampleStart = int.from_bytes(file.read(4), byteorder='little')
				cues.append(sampleStart)
			
			return cues
		else:
			file.seek(padded_size, os.SEEK_CUR)

	return []

def check_cues(file):
	wav = None
	cue_points = get_wave_cue_points(file)
	try:
		wav = wave.open(file, 'rb')
		
		if len(cue_points) == 1 and cue_points[0] > wav.getnframes()/2:
			print("invalid cue point: %s", cue_points)
		elif len(cue_points) > 1 and cue_points[0] > cue_points[1]:
			print("inverted cue points: %s", cue_points)
		else:
			print("cue points: %s" % cue_points)
		
		wav.close()
	except Exception as e:
		if 'unknown format' not in '%s' % e:
			raise e		
		else:
			print(e)

def add_wave_cue_points(fpath, cue_points):
	
	wav = wave.open(fpath, 'rb')
	print("WAV Sample Rate %s" % wav.getframerate())
	
	new_cues = []
	for cue in cue_points:
		t = int(cue * wav.getframerate())
		new_t = min(t, wav.getnframes()-1)
		new_cues.append(new_t)
		print("Add cue %ss -> %s samples" % (cue, new_t))
		
	cue_points = new_cues
	
	wav.close()
	
	with open(fpath, 'r+b') as file:
		file.seek(0, os.SEEK_END)
		
		file.write(b'cue ')
		file.write(struct.pack('<I', len(cue_points)*24))
		file.write(struct.pack('<I', len(cue_points)))
		
		for idx, cue in enumerate(cue_points):
			file.write(struct.pack('<I', idx))
			file.write(struct.pack('<I', 0)) # position
			file.write(b'data')
			file.write(struct.pack('<I', 0)) # chunk start
			file.write(struct.pack('<I', 0)) # block start
			file.write(struct.pack('<I', cue)) # sample start
			
		newSize = file.tell() - 8 # sub the WAVE header
		
		# update file size
		file.seek(4, os.SEEK_SET)
		file.write(struct.pack('<I', newSize))

def remove_wav_cue_points(fpath):
	with open(fpath, "rb") as f:
		data = f.read()

	if data[:4] != b"RIFF" or data[8:12] != b"WAVE":
		raise ValueError("Not a WAV file")

	out = bytearray()

	# Copy RIFF header (size fixed later)
	out += data[:12]

	pos = 12
	while pos + 8 <= len(data):
		chunk_id = data[pos:pos+4]
		chunk_size = struct.unpack("<I", data[pos+4:pos+8])[0]

		# Chunk data starts here
		chunk_end = pos + 8 + chunk_size

		# Chunks are padded to even length
		padded_end = chunk_end + (chunk_size & 1)

		keep = True

		if chunk_id == b"cue ":
			print("Removed cue chunk")
			keep = False

		elif chunk_id == b"plst":
			print("Removed plst chunk")
			keep = False

		elif chunk_id == b"LIST":
			# LIST begins with a 4-byte type
			if chunk_size >= 4:
				list_type = data[pos+8:pos+12]
				if list_type == b"adtl":
					print("Removed LIST chunk")
					keep = False

		if keep:
			out += data[pos:padded_end]

		pos = padded_end

	# Update RIFF size
	struct.pack_into("<I", out, 4, len(out) - 8)

	with open(fpath, "wb") as f:
		f.write(out)

if len(sys.argv) < 2:
	print ("Usage: cue_points.py <file> [cue_start] [cue_end]")
	print("\nLists cue points saved in the file. Cue points make sounds loop automatically in HL.")
	print("If cue_start and cue_end are set, cue points are added to the file. Values are given in seconds.")
	print("To remove cue points, set start and end cues to 0.")
	print("\nExamples:")
	print("cue_points.py test.wav 0 0         = remove all cue points")
	print("cue_points.py test.wav 0.5 1       = loop from 0.5s to 1s")
	print("cue_points.py test.wav 0 999       = loop the entire sound")
	sys.exit()

fpath = sys.argv[1]
if not os.path.exists(fpath):
	print("File does not exist: %s" % fpath)

if len(sys.argv) < 4:
	check_cues(fpath)
else:
	c1 = float(sys.argv[2])
	c2 = float(sys.argv[3])
	add_cues = [min(c1, c2), max(c1, c2)] # don't invert cues or else client crashes

	if add_cues[0] > 0 or add_cues[1] > 0:
		remove_wav_cue_points(fpath)
		add_wave_cue_points(fpath, add_cues)
	else:
		remove_wav_cue_points(fpath)
