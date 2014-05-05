angular.module("app").controller "TransactionsController", ($scope, $location, RpcService) ->

  $scope.rescan = ->
    RpcService.request("get_transaction_history").then (response) ->
      console.log "--- transactions = ", response
      $scope.transactions = response

  $scope.rescan()