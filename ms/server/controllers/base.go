package controllers

import (
	"time"

	"github.com/astaxie/beego"
)

type BaseController struct {
	beego.Controller
}

func JsonFormat(code int, retmsg string, retdata interface{}, stime time.Time) (json map[string]interface{}) {
	cost := time.Now().Sub(stime).Seconds()
	if code == 0 {
		json = map[string]interface{}{
			"code": code,
			"data": retdata,
			"desc": retmsg,
			"cost": cost,
		}
	} else {
		json = map[string]interface{}{
			"code": code,
			"desc": retmsg,
			"cost": cost,
		}
	}

	return json
}
