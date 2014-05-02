app = angular.module("app", [
  "ngResource"
  "ngRoute"
])
app.config ($routeProvider) ->
  $routeProvider.when "/",
    templateUrl: "home.html"
    controller: "HomeController"

  $routeProvider.when "/recieve",
    templateUrl: "receive.html"
    controller: "ReceiveController"

  $routeProvider.when "/transfer",
    templateUrl: "transfer.html"
    controller: "TransferController"

  $routeProvider.otherwise redirectTo: "/"

  return
