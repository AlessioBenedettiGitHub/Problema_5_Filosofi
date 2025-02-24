import matplotlib.pyplot as plt
import numpy as np

def readf(filename):
  with open(filename) as f:
    contents = f.read()
    contents = contents.splitlines()
    Nturns = len(contents)
    Nfilo =int(len((contents[1]).split())/3)
    cycldep = int(contents[0])
    PosataSX = []
    PosataDX = []
    Desina = []
    for ct in range(1,Nturns):
      temp=[int(x) for x in contents[ct].split()]
      PosataSX.append(temp[0:Nfilo])
      PosataDX.append(temp[Nfilo:Nfilo*2])
      Desina.append(temp[2*Nfilo:Nfilo*3])
  return cycldep, PosataSX, PosataDX, Desina


# parte "MAIN" del programma
filename = "outdata.txt"
print("Leggo il file....")
# estraggo una matrice di interi
[cycldep,PosataSX, PosataDX, Desina] = readf(filename)
Nfilo=len(PosataSX[0])
print("\n I filosofi a cena sono "+str(Nfilo))
print("\n I dati coprono " + str(len(PosataSX)) + " cicli.\n")

phi = np.linspace(0, 2*np.pi, 50)
h_r=1      # head radius
t_r=8+(Nfilo-5)*h_r  # table seat radius
bt_r=7.5+(Nfilo-5)*h_r  # table border radius
fo_l=1.5          # fork length
fo_r=5+(Nfilo-5)*h_r            # fork mean radial position
d_r=1.2            # dish radius

ct=0
while (ct<len(PosataSX)):
  
  
  plt.title(str(Nfilo)+' filosofi a cena')

  # tavola
  x = bt_r*np.cos(phi)
  y = bt_r*np.sin(phi)
  plt.fill(x, y, edgecolor='cyan', facecolor='cyan', zorder=1)
  
  for nF in range(0,Nfilo):
    # spalle
    angle=2*np.pi/Nfilo*nF
    xt = 2.2*h_r*np.cos(phi)
    yt = t_r+0.9*h_r*np.sin(phi)
    x=xt*np.cos(angle)-yt*np.sin(angle)
    y=xt*np.sin(angle)+yt*np.cos(angle)
    if ((nF==Nfilo-1)&(cycldep==0)):
        plt.fill(x, y, edgecolor='blue', facecolor='blue',zorder=2)
    else:
        plt.fill(x, y, edgecolor='red', facecolor='red',zorder=2)

    # testa
    angle=np.pi/2+2*np.pi/Nfilo*nF
    x = t_r*np.cos(angle)+h_r*np.cos(phi)
    y = t_r*np.sin(angle)+h_r*np.sin(phi)
    if ((nF==Nfilo-1)&(cycldep==0)):
      plt.fill(x, y, edgecolor='yellow', facecolor='yellow',zorder=3)
    else:
      plt.fill(x, y, edgecolor='blue', facecolor='blue',zorder=3)


    # forchetta
    x = np.array([(fo_r-fo_l/2)*np.cos(angle+2*np.pi/(2*Nfilo)),(fo_r+fo_l/2)*np.cos(angle+2*np.pi/(2*Nfilo))])
    y = np.array([(fo_r-fo_l/2)*np.sin(angle+2*np.pi/(2*Nfilo)),(fo_r+fo_l/2)*np.sin(angle+2*np.pi/(2*Nfilo))])
    plt.plot(x, y, linewidth=3, color='yellow',zorder=2)

    if (PosataSX[ct][nF]==1):
      # braccio sinistro
      angle=2*np.pi/Nfilo*nF
      xt = np.array([fo_r*np.sin(2*np.pi/(2*Nfilo)),h_r*2.2])
      yt = np.array([fo_r*np.cos(2*np.pi/(2*Nfilo)),t_r-h_r*0.5])
      x=xt*np.cos(angle)-yt*np.sin(angle)
      y=xt*np.sin(angle)+yt*np.cos(angle)
      plt.plot(x, y, linewidth=5, color='green', zorder=3)

    if (PosataDX[ct][nF]==1):
      # braccio destro
      angle=2*np.pi/Nfilo*nF
      xt = np.array([-h_r*2.2,-fo_r*np.sin(2*np.pi/(2*Nfilo))])
      yt = np.array([t_r-h_r*0.5,fo_r*np.cos(2*np.pi/(2*Nfilo))])
      x=xt*np.cos(angle)-yt*np.sin(angle)
      y=xt*np.sin(angle)+yt*np.cos(angle)
      plt.plot(x, y, linewidth=5, color='green', zorder=3)
  
    if (Desina[ct][nF]==1):
      # piatto
      angle=np.pi/2+2*np.pi/Nfilo*nF
      x = (fo_r-h_r/2)*np.cos(angle)+d_r*np.cos(phi)
      y = (fo_r-h_r/2)*np.sin(angle)+d_r*np.sin(phi)
      plt.fill(x, y, edgecolor='white', facecolor='white',zorder=3)
      x = (fo_r-h_r/2)*np.cos(angle)+2/5*d_r*np.cos(phi)
      y = (fo_r-h_r/2)*np.sin(angle)+2/5*d_r*np.sin(phi)
      plt.fill(x, y, edgecolor='cyan', facecolor='cyan', zorder=4)

  plt.axis('equal')
  plt.draw()
  plt.pause(5e-1)
  plt.clf()
  ct=ct+1

print("Fatto!")
