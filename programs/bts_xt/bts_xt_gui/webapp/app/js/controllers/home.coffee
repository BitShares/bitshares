angular.module("app").controller "HomeController", ($scope, $modal, $log, RpcService) ->

  $scope.balance = 0.0

#  RpcService.request('current_wallet').then (response) ->
#    if response.result == null
#      console.log "no current_wallet, please open wallet"
#      $scope.open_wallet()

  RpcService.request('getbalance').then (response) ->
    console.log "balance: ", response.result.amount
    $scope.balance = response.result.amount

