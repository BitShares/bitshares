angular.module("app.directives", []).directive "alertBar", ($parse) ->
  restrict: "A"
  template: """
  <div class="alert alert-danger">
    <button type="button" class="close" data-dismiss="alert" aria-hidden="true" ng-click="hideAlert()" >×</button>
    <i class="fa fa-exclamation-circle"></i>
    {{errorMessage}}
   </div> """
  link: (scope, elem, attrs) ->
    alertMessageAttr = attrs["alertmessage"]
    scope.errorMessage = null

    scope.$watch alertMessageAttr, (newVal) ->
      scope.errorMessage = newVal
      scope.isHidden = !newVal

    scope.hideAlert = ->
      scope.errorMessage = null
      $parse(alertMessageAttr).assign scope, null
      scope.isHidden = true
