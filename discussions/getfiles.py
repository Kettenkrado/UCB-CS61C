for i in range(1, 13):
  if i < 10:
    filename = "disc0" + str(i)
  else:
    filename = "disc" + str(i)

  print("curl -O https://cs61c.org/su24/pdfs/discussions/" + filename + ".pdf",
        sep="")
  print("curl -O https://cs61c.org/su24/pdfs/discussions/" + filename +
        "-sols.pdf",
        sep="")
