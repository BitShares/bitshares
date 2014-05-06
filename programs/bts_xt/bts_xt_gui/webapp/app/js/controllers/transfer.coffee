angular.module("app").controller "TransferController", ($scope, $location, $state, RpcService) ->

  $scope.send = ->
    RpcService.request('sendtoaddress', [$scope.payto,$scope.amount,$scope.memo]).then (response) ->
      $state.go("transactions")
