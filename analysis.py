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
        # static member?
        self.__mpion = 139 # MeV
        self.__filter_pdgid = 211
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
            # self.__miniloop(batch.keys())
            # select pions
            pdgid_mask = batch["track.pdgid"] == self.__filter_pdgid
            selected = batch[pdgid_mask]
            selected_pdgid = batch[pdgid_mask]['track.pdgid']
            # from here, I always assume one particle in one event
            print(selected_pdgid[:, 0])

            # analyzed = nevent * [
            # nstep *
            # [dE, dE/dz, dE/dr, dz, px, py, pz, x, y, z, Ek, volume],
            # z_stop, init_px, init_py, init_pz, init_Ek, init_p]

            # [ievent, iparticle, isteps]
            steps = {}
            for d in ["x", "y", "z"]:
                pre_loc = selected["track.post_{}".format(d)][:, 0, 0:-1]
                pos_loc = selected["track.post_{}".format(d)][:, 0, 1:]
                steps["diff_{}".format(d)] = pos_loc - pre_loc
                steps["pos_{}".format(d)] = pos_loc

                steps["pos_p{}".format(d)] = selected[
                    "track.post_p{}".format(d)][:, 0, 1:]

                steps["init_p{}".format(d)] = selected[
                    "track.post_p{}".format(d)][:, 0, 0]

            steps["diff_r"] = np.sqrt(
                steps["diff_x"]**2 + steps["diff_y"]**2
                + steps["diff_z"]**2
            )

            steps["pos_KE"] = np.sqrt(
                steps["pos_px"]**2 + steps["pos_py"]**2
                + steps["pos_pz"]**2 + self.__mpion**2
            ) - self.__mpion

            steps["init_KE"] = np.sqrt(
                steps["init_px"]**2 + steps["init_py"]**2
                + steps["init_pz"]**2 + self.__mpion**2
            ) - self.__mpion

            steps["init_p"] = np.sqrt(
                steps["init_px"]**2 + steps["init_py"]**2
                + steps["init_pz"]**2
            )

            steps["volume"] = selected["track.volume"][:, 0, 1:]
            steps["dE"] = selected["track.edep"][:, 0, 1:]

            # not sure how to deal with dz=0
            # steps["dE/dz"] = steps["dE"]/steps["diff_z"]
            steps["z_stop"] = selected["track.post_z"][:, 0, -1]

            analyzed = ak.Array(steps)
            # print(analyzed)
            # for event in analyzed:
            #     print(event.tolist())

    # @staticmethod
    # @nb.njit
    # def __miniloop(batch):
    #     """loop over a batch
    #     """
    #     for event in batch:
    #         self.__single_event(event)

    # @staticmethod
    # @nb.njit
    # def __single_event(event):
    #     """Process single event
    #     """
    #     return
