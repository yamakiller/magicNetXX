package controllers

import (
	"server/models"
	"time"

	"github.com/dchest/captcha"
)

// Operations about Users
type AccountController struct {
	BaseController
}

// @Title Get
// @Description get user by uid
// @Param	uid		path 	string	true		"The key for staticblock"
// @Success 200 {object} models.User
// @Failure 403 :uid is empty
// @router /:uid [get]
func (u *AccountController) Get() {
	//uid := u.GetString(":uid")

	u.ServeJSON()
}

// @Title Update
// @Description update the user
// @Param	uid		path 	string	true		"The uid you want to update"
// @Param	body		body 	models.User	true		"body for user content"
// @Success 200 {object} models.User
// @Failure 403 :uid is not int
// @router /:uid [put]
func (u *AccountController) Put() {
	//uid := u.GetString(":uid")
	u.ServeJSON()
}

// @Title Delete
// @Description delete the user
// @Param	uid		path 	string	true		"The uid you want to delete"
// @Success 200 {string} delete success!
// @Failure 403 uid is empty
// @router /:uid [delete]
func (u *AccountController) Delete() {
	//uid := u.GetString(":uid")
	u.ServeJSON()
}

// @Title Login
// @Description Logs user into the system
// @Param	username		query 	string	true		"The username for login"
// @Param	password		query 	string	true		"The password for login"
// @Success 200 {string} login success
// @Failure 403 user not exist
// @router /login [get]
func (u *AccountController) Login() {
	username := u.GetString("username")
	password := u.GetString("password")
	captchaId := u.GetString("captchaId")
	captchaValue := u.GetString("captcha")

	if !IsCheckAccount(username) {
		u.Data["result"] = JsonFormat(1, "user or password error", "", time.Now())
		u.ServeJSON()
		return
	}

	if !captcha.VerifyString(captchaId, captchaValue) {
		u.Data["result"] = JsonFormat(1, "verify code error", "", time.Now())
		u.ServeJSON()
		return
	}

	if models.Login(username, password) {
		u.Data["json"] = "login success"
	} else {
		u.Data["json"] = "user not exist"
	}
	u.ServeJSON()
}

// @Title logout
// @Description Logs out current logged in user session
// @Success 200 {string} logout success
// @router /logout [get]
func (u *AccountController) Logout() {
	u.Data["json"] = "logout success"
	u.ServeJSON()
}
