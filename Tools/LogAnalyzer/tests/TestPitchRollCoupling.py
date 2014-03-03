from LogAnalyzer import Test,TestResult
import DataflashLog


class TestPitchRollCoupling(Test):
	'''test for divergence between input and output pitch/roll, i.e. mechanical failure or bad PID tuning'''
	# TODO: currently we're only checking for roll/pitch outside of max lean angle, will come back later to analyze roll/pitch in versus out values

	def __init__(self):
		self.name = "Pitch/Roll"
		self.enable = True   # TEMP

	def run(self, logdata, verbose):
		self.result = TestResult()
		self.result.status = TestResult.StatusType.PASS

		if logdata.vehicleType != "ArduCopter":
			self.result.status = TestResult.StatusType.NA
			return

		if not "ATT" in logdata.channels:
			self.result.status = TestResult.StatusType.UNKNOWN
			self.result.statusMessage = "No ATT log data"
			return

		# figure out where each mode begins and ends, so we can treat auto and manual modes differently and ignore acro/tune modes
		autoModes   = ["RTL","AUTO","LAND","LOITER","GUIDED","CIRCLE","OF_LOITER"]     # use NTUN DRol+DPit
		manualModes = ["STABILIZE","DRIFT","ALT_HOLD"]                                 # use CTUN RollIn/DesRoll + PitchIn/DesPitch
		ignoreModes = ["ACRO","SPORT","FLIP","AUTOTUNE"]                               # ignore data from these modes
		autoSegments   = []  # list of (startLine,endLine) pairs
		manualSegments = []  # list of (startLine,endLine) pairs
		orderedModes = collections.OrderedDict(sorted(logdata.modeChanges.items(), key=lambda t: t[0]))
		isAuto = False # we always start in a manual control mode
		prevLine = 0
		for line,modepair in orderedModes.iteritems():
			mode = modepair[0].upper()
			if prevLine == 0:
				prevLine = line
			if mode in autoModes:
				if not isAuto:
					manualSegments.append((prevLine,line-1))
					#print "Adding manual segment: %d,%d" % (prevLine,line-1)
					prevLine = line
				isAuto = True
			elif mode in manualModes:
				if isAuto:
					autoSegments.append((prevLine,line-1))
					#print "Adding auto segment: %d,%d" % (prevLine,line-1)
					prevLine = line
				isAuto = False
			elif mode in ignoreModes:
				if isAuto:
					autoSegments.append((prevLine,line-1))
					#print "Adding auto segment: %d,%d" % (prevLine,line-1)
				else:
					manualSegments.append((prevLine,line-1))
					#print "Adding manual segment: %d,%d" % (prevLine,line-1)
				prevLine = 0
			else:
				raise Exception("Unknown mode in TestPitchRollCoupling: %s" % mode)
		# and handle the last segment, which doesn't have an ending
		if mode in autoModes:
			autoSegments.append((prevLine,logdata.lineCount))
			#print "Adding final auto segment: %d,%d" % (prevLine,logdata.lineCount)
		elif mode in manualModes:
			manualSegments.append((prevLine,logdata.lineCount))
			#print "Adding final manual segment: %d,%d" % (prevLine,logdata.lineCount)

		# figure out max lean angle, the ANGLE_MAX param was added in AC3.1
		maxLeanAngle = 45.0
		if "ANGLE_MAX" in logdata.parameters:
			maxLeanAngle = logdata.parameters["ANGLE_MAX"] / 100.0
		maxLeanAngleBuffer = 10 # allow a buffer margin

		# ignore anything below this altitude, to discard any data while not flying
		minAltThreshold = 2.0

		# look through manual+auto flight segments
		# TODO: filter to ignore single points outside range?
		(maxRoll, maxRollLine)   = (0.0, 0)
		(maxPitch, maxPitchLine) = (0.0, 0)
		for (startLine,endLine) in manualSegments+autoSegments:
			#print "Checking segment %d,%d" % (startLine,endLine)
			# quick up-front test, only fallover into more complex line-by-line check if max()>threshold
			rollSeg  = logdata.channels["ATT"]["Roll"].getSegment(startLine,endLine)
			pitchSeg = logdata.channels["ATT"]["Pitch"].getSegment(startLine,endLine)
			if not rollSeg.dictData and not pitchSeg.dictData:
				continue
			# check max roll+pitch for any time where relative altitude is above minAltThreshold
			roll  = max(abs(rollSeg.min()),  abs(rollSeg.max()))
			pitch = max(abs(pitchSeg.min()), abs(pitchSeg.max()))
			if (roll>(maxLeanAngle+maxLeanAngleBuffer) and abs(roll)>abs(maxRoll)) or (pitch>(maxLeanAngle+maxLeanAngleBuffer) and abs(pitch)>abs(maxPitch)):
				lit = DataflashLog.LogIterator(logdata, startLine)
				assert(lit.currentLine == startLine)
				while lit.currentLine <= endLine:
					relativeAlt = lit["CTUN"]["BarAlt"]
					if relativeAlt > minAltThreshold:
						roll  = lit["ATT"]["Roll"]
						pitch = lit["ATT"]["Pitch"]
						if abs(roll)>(maxLeanAngle+maxLeanAngleBuffer) and abs(roll)>abs(maxRoll):
							maxRoll = roll
							maxRollLine = lit.currentLine
						if abs(pitch)>(maxLeanAngle+maxLeanAngleBuffer) and abs(pitch)>abs(maxPitch):
							maxPitch = pitch
							maxPitchLine = lit.currentLine
					lit.next()
		# check for breaking max lean angles
		if maxRoll and abs(maxRoll)>abs(maxPitch):
			self.result.status = TestResult.StatusType.FAIL
			self.result.statusMessage = "Roll (%.2f, line %d) > maximum lean angle (%.2f)" % (maxRoll, maxRollLine, maxLeanAngle)
			return
		if maxPitch:
			self.result.status = TestResult.StatusType.FAIL
			self.result.statusMessage = "Pitch (%.2f, line %d) > maximum lean angle (%.2f)" % (maxPitch, maxPitchLine, maxLeanAngle)
			return



		# TODO: use numpy/scipy to check Roll+RollIn curves for fitness (ignore where we're not airborne)
		# ...







