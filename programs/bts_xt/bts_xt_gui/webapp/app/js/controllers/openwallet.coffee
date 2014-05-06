angular.module("app").controller "OpenWalletController", ($scope, $modalInstance, RpcService, ErrorService, mode) ->
  console.log "OpenWalletController mode: #{mode}"
  if mode == "open_wallet"
    $scope.title = "Open Wallet"
    $scope.password_label = "Wallet Password"
    $scope.wrong_password_msg = "Wallet cannot be opened. Please check you password"
  else if mode == "unlock_wallet"
    $scope.title = "Unlock Wallet"
    $scope.password_label = "Spending Password"
    $scope.wrong_password_msg = "Wallet cannot be unlocked. Please check you password"
  else
    ErrorService.setError "OpenWalletController unknown mode: #{mode}"
    $modalInstance.dismiss()

  open_wallet_request = ->
    RpcService.request('open_wallet', ['default', $scope.password]).then (response) ->
      console.log "--------", response, response.data
      if response.result == true || response.data?.result == true
        $modalInstance.close("ok")
      else
        $scope.password_validation_error()
    ,
    (reason) ->
      console.log "-------- error", reason
      $scope.password_validation_error()

  unlock_wallet_request = ->
    RpcService.request('walletpassphrase', [$scope.password, 60 * 1000000]).then (response) ->
      $modalInstance.close("ok")
    ,
    (reason) ->
      $scope.password_validation_error()

  $scope.has_error = false
  $scope.ok = ->
    if mode == "open_wallet" then open_wallet_request() else unlock_wallet_request()

  $scope.password_validation_error = ->
    $scope.password = ""
    $scope.has_error = true

  $scope.cancel = ->
    $modalInstance.dismiss "cancel"

  $scope.$watch "password", (newValue, oldValue) ->
    $scope.has_error = false if newValue != ""