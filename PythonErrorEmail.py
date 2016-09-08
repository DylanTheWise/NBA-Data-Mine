import keyring
import smtplib

def Main():
	msg = "\r\n".join([
		"From: ukystatisticsbot@gmail.com",
		"To: dmwi235@g.uky.edu, czal2222@gmail.com, joshua.lambert@uky.edu",
		"Subject: NBA stats update script has stopped",
		"",
		"The script has stopped running and needs to be restarted."
		])
		
	server = smtplib.SMTP('smtp.gmail.com:587')
	server.ehlo()
	server.starttls()
	
	
	server.login('ukystatisticsbot@gmail.com', keyring.get_password('MailBot','ukystatisticsbot@gmail.com'))
	server.sendmail("ukystatisticsbot@gmail.com", ['dmwi235@g.uky.edu','czal2222@gmail.com', 'joshua.lambert@uky.edu'], msg)
	server.quit()

Main()