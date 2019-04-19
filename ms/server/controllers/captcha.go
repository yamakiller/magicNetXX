package controllers

import (
	"github.com/astaxie/beego"
	"github.com/dchest/captcha"
)

type CaptchaController struct {
	beego.Controller
}

func (u *CaptchaController) GetImage() {
	d := struct {
		CaptchaId string
	}{
		captcha.New(),
	}

	u.Data["CaptchaId"] = d.CaptchaId
	u.ServeJSON()
}
