import numpy as np
import uproot
f = uproot.open("michel_energy_spectrum.root")
h_e_michel = f["h_e_michel"]
fracs = h_e_michel.values() / np.sum(h_e_michel.values())
centers = h_e_michel.axis().centers()
widths = h_e_michel.axis().widths()
output = np.array([centers, widths, fracs])
output = output.transpose()
np.savetxt("mutoe_michel_spectrum.txt", output)
