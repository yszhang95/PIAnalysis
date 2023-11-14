import numba as nb
import numpy as np
import awkward as ak

import uproot
# import hist
from hist import Hist

import matplotlib
import mplhep




class HistFactory:
    """
    Histogram factory
    """

    def __init__(self):
        self.__flist = []
        self.__tname = "sim"
        # self.__info_tname = "info"
        self.__track_tname = "track"
        # beam momentum, 1D, 0--100 MeV, 100 bins
        self.__h_beam_mom = (
            Hist.new.Reg(100, 0, 100,
                         name="beam momentum (MeV)").Double()
        )
        # beam kinetic energy, 1D, 0--80 MeV, 80 bins
        self.__h_beam_ke = (
            Hist.new.Reg(100, 0, 100, name="x",
                         label="beam kinetic energy (MeV)").Double()
        )

        # beam incident angle in xy, -pi--pi, 50
        self.__h_beam_phi = (
            Hist.new.Reg(50, -np.pi, np.pi, name="x",
                         label="beam incident angle in xy (rad)").Double()
        )
        # beam incident angle in xz, -pi--pi, 50
        self.__h_beam_theta = (
            Hist.new.Reg(50, -np.pi, np.pi, name="x",
                         label="beam incident angle in xz (rad)").Double()
        )
        # dE/dr along trajectory vs. z, 2D histogram
        self.__h_dedr_vs_z = (
            Hist.new.Reg(50, -1, 9, name="x", label="z (mm)")
            .Reg(50, 0, 100, name="y", label="dE/dr (MeV/cm)")
            .Double()
        )
        # dE/dr along z vs. z, 2D histogram
        self.__h_dedz_vs_z = (
            Hist.new.Reg(50, -1, 9, name="x", label="z (mm)")
            .Reg(50, 0, 100, name="y", label="dE/dz (MeV/cm)")
            .Double()
        )
        # stopping point along z, 1D histogram
        self.__h_stop_z = (
            Hist.new.Reg(100, -1, 9, name="x",
                         label="z of stopping point (mm)").Double()
        )

    @property
    def input_list(self):
        """Getter
        """
        return self.__flist

    @input_list.setter
    def input_list(self, inlist):
        """Setter"""
        self.__flist = inlist

    def plot(self) -> None:
        """
        plot
        """
        self.__h_beam_mom.plot()
        self.__h_beam_ke.plot()
        self.__h_beam_phi.plot()
        self.__h_beam_theta.plot()
        self.__h_dedr_vs_z.plot()
        self.__h_dedz_vs_z.plot()
        self.__h_stop_z.plot()

    def loop(self) -> None:
        """
        loop
        """
        f_iter = uproot.iterate(["{f}:{t}/{subt}"
                                 .format(f=f, t=self.__tname,
                                         subt=self.__track_tname)
                                 for f in self.__flist])
        for batch in f_iter:
            self.__miniloop(batch)

    @staticmethod
    @nb.njit
    def __miniloop(batch):
        """loop over a batch
        """
        for event in batch:
            print(event)
