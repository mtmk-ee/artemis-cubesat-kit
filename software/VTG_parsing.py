def _parse_GPVTG (self, args)
    data = args.split(',')
    if data is None:
        return
    
    self.truetrack = _parse_degrees(data[0])
    self.tracknorth = data[1] #shpuld be 'T'-relative to true north
    self.magnetictrack = data[2]
    self.magneticnorth = data[3] #should be 'M'-relative to magnetic north
    self.groundspeedn = data[4]
    self.knots = data[5] #should be 'N'-knots
    self.groundspeedk = data[6]
    self.kph = data[7] #should be 'K'-kilometers per hour
