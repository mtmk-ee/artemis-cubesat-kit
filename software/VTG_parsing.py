def _parse_GPVTG (self, args)
    data = args.split(',')
    if data is None:
        return
    
    self.Tcourse = _parse_float(data[0])
    self.Treference = _parse_str(data[1])
    self.Mcourse = _parse_float(data[2])
    self.Mreference = _parse_str(data[3])
    self.Ncourse = _parse_float(data[4])
    self.Nreference = _parse_str(data[5])
    self.Kcourse = _parse_float(data[6])
    self.Kreference = _parse_str(data[7])
