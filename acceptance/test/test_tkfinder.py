import numpy

import ROOT

from sklearn.decomposition import PCA

points = numpy.random.multivariate_normal([0, 0, 0], [[7, 3.464, 0],
                                                   [3.464, 3, 0], [0, 0, 10]],
                                          size=5000)

prin = ROOT.PIAna.PITkPCA(3)
for p in points:
    prin.add_data(p)

prin.fit()

prin.add_data(points[0])

dire = prin.get_direction()

print("XYZ vector", dire.X(), dire.Y(), dire.Z())
print("Polar vector", dire.R(), dire.Theta(), dire.Phi())

prin.clear_data()

prin.fit()

pca = PCA(3)
pca.fit(points)
print(pca.components_)
