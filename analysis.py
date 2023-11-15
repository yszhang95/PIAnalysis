import numba as nb
import numpy as np
import awkward as ak
from hepunits import units as u

import uproot
# import hist
from hist import Hist

import matplotlib
from matplotlib import pyplot as plt
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
            Hist.new.Reg(100, 0, 100, name="beam_mom_x",
                         label="beam momentum (MeV)").Double()
        )
        # beam kinetic energy, 1D, 0--80 MeV, 80 bins
        self.__h_beam_ke = (
            Hist.new.Reg(100, 0, 100, name="beam_ke_x",
                         label="beam kinetic energy (MeV)").Double()
        )

        # beam incident angle in xy, -pi--pi, 50
        self.__h_beam_phi = (
            Hist.new.Reg(50, -np.pi, np.pi, name="beam_phi_x",
                         label="beam incident angle in xy (rad)").Double()
        )
        # beam incident angle in xz, -pi--pi, 50
        self.__h_beam_theta = (
            Hist.new.Reg(50, -np.pi, np.pi, name="beam_theta_x",
                         label="beam incident angle in xz (rad)").Double()
        )
        # dE/dr along trajectory vs. z, 2D histogram
        self.__h_dedr_vs_z = (
            Hist.new.Reg(50, -1, 9, name="dedr_vs_z_x", label="z (mm)")
            .Reg(50, 0, 100, name="dedr_vs_z_y", label="dE/dr (MeV/cm)")
            .Double()
        )
        # dE/dr along z vs. z, 2D histogram
        self.__h_dedz_vs_z = (
            Hist.new.Reg(50, -1, 9, name="dedz_vs_z_x", label="z (mm)")
            .Reg(50, 0, 100, name="dedz_vs_z_y", label="dE/dz (MeV/cm)")
            .Double()
        )
        # stopping point along z, 1D histogram
        self.__h_stop_z = (
            Hist.new.Reg(100, -1, 9, name="stop_z_x",
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
        print(self.__h_dedr_vs_z)
        mplhep.style.use("CMS")
        self.__h_beam_mom.plot()
        self.__h_beam_ke.plot()
        self.__h_beam_phi.plot()
        self.__h_beam_theta.plot()
        mplhep.histplot(self.__h_beam_theta)
        plt.savefig("beam_theta.png")
        mplhep.hist2dplot(self.__h_dedr_vs_z,
                          norm=matplotlib.colors.LogNorm(vmin=0.1))
        #self.__h_dedr_vs_z.plot()
        plt.savefig("dedr_vs_z.png")
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

            steps["theta"] = np.arccos(steps["init_pz"]/
                                       steps["init_p"])

            analyzed = ak.Array(steps)

            # fill incident angle
            self.__h_beam_theta.fill(analyzed["theta"])

            # silicon bulk
            silicon_bulk_mask = np.logical_and(
                analyzed["volume"] <= 109_999,
                analyzed["volume"] >= 100_000
            )

            # silicon bulk + theta < 0.01
            shoot_in_atar = np.logical_and(
                ak.any(silicon_bulk_mask, axis=1),
                analyzed["theta"] < 0.1
            )

            filtered_analyzed = analyzed[ak.any(
                silicon_bulk_mask, axis=1)]

            silicon_bulk_mask_updated = np.logical_and(
                filtered_analyzed["volume"] <= 109_999,
                filtered_analyzed["volume"] >= 100_000
            )
            # dz > 0
            dz_mask = np.logical_and(
                np.abs(filtered_analyzed["diff_z"]) > 0,
                silicon_bulk_mask_updated
            )

            # dr > 0
            dr_mask = np.logical_and(
                np.abs(filtered_analyzed["diff_r"]) > 0,
                silicon_bulk_mask_updated
            )

            dedz = (
                filtered_analyzed["dE"][dz_mask]/
                filtered_analyzed["diff_z"][dz_mask]
            ) / u.mm * u.cm

            dedr = (
                filtered_analyzed["dE"][dr_mask]/
                filtered_analyzed["diff_r"][dr_mask]
                    ) / u.mm * u.cm

            self.__h_dedz_vs_z.fill(dedz_vs_z_x=ak.flatten(
                filtered_analyzed["pos_z"][dz_mask]),
                                    dedz_vs_z_y=ak.flatten(dedz))

            # print("small dEdx",np.sum(dedr<1E-3))
            # print("large dEdx", len(ak.flatten(dedr)))
            # print(filtered_analyzed["volume"][dr_mask][dedr<1E-3])


            self.__h_dedr_vs_z.fill(dedr_vs_z_x=ak.flatten(
                filtered_analyzed["pos_z"][dr_mask]),
                                    dedr_vs_z_y=ak.flatten(dedr))

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
