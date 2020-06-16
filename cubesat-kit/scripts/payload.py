
import cosmos

print("======== Loading payload.py ========")



#cosmos.tempsensor.GetTemp("plus_x")

def Update():
	print("payload.py update")





print(cosmos.SetUpdateCallback("payload", Update, 1))
