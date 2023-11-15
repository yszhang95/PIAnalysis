from analysis import HistFactory

h = HistFactory()
h.input_list = ["stable_pi_test_Mom55MeV.root"]
h.loop()
h.plot()
h.save_to_file()
