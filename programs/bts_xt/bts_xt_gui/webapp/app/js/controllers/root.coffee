angular.module("app").controller "RootController", ($scope, $location, $modal, ErrorService) ->
  $scope.errorService = ErrorService

  $scope.open_wallet = ->
    $modal.open
      templateUrl: "openwallet.html"
      controller: "OpenWalletController"
      resolve: { mode: -> "open_wallet" }

  $scope.unlock_wallet = ->
    $modal.open
      templateUrl: "openwallet.html"
      controller: "OpenWalletController"
      resolve: { mode: -> "unlock_wallet" }

  $scope.$on 'event:walletOpenRequired', ->
    console.log "------ event:walletOpenRequired ------"
    $scope.open_wallet()

  $scope.$on 'event:walletUnlockRequired', ->
    console.log "------ event:walletUnlockRequired ------"
    $scope.unlock_wallet()