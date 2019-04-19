package models

type Account struct {
	Id   string
	Name string
	Pass string
}

func Login(username, password string) bool {

	return false
}

func DeleteUser(uid string) {
}

func IsCheckAccount(Name string) bool {
	if Name == "" {
		return false
	}

	return true
}

func IsCheckPassword(Pass string) bool {
	if Pass == "" {
		return false
	}

	return true
}
