// @APIVersion 1.0.0
// @Title beego Test API
// @Description beego has a very cool tools to autogenerate documents for your API
// @Contact astaxie@gmail.com
// @TermsOfServiceUrl http://beego.me/
// @License Apache 2.0
// @LicenseUrl http://www.apache.org/licenses/LICENSE-2.0.html
package routers

import (
	"server/controllers"

	"github.com/astaxie/beego"
	"github.com/dchest/captcha"
)

func init() {
	ns := beego.NewNamespace("/api",
		beego.NSNamespace("/account",
			beego.NSInclude(
				&controllers.AccountController{},
			),
		),
	)

	checkImageW, _ := beego.AppConfig.Int("check_image_width")
	checkImageH, _ := beego.AppConfig.Int("check_image_height")

	beego.Handler("/captcha/*.png", captcha.Server(checkImageW, checkImageH))
	beego.AddNamespace(ns)
}
