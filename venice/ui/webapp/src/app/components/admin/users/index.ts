import { UsersComponent } from './users.component';
import { UsersModule } from './users.module';


import { UIConfigsService } from '@app/services/uiconfigs.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { ControllerService } from '@app/services/controller.service';

export interface UserDataReadyMap {
    users: boolean;
    roles: boolean;
    rolebindings: boolean;
}

export const hasStagingPermission = (uiconfigsService: UIConfigsService): boolean => {
    if (uiconfigsService.isAuthorized(UIRolePermissions.stagingbuffer_create)
    && uiconfigsService.isAuthorized(UIRolePermissions.stagingbuffer_commit)
    && uiconfigsService.isAuthorized(UIRolePermissions.stagingbuffer_delete)
   ) {
       return true;
   } else {
       return false;
   }
  };

export const invokeConfigureStagingPermission = (_controllerService: ControllerService)  => {
    _controllerService.invokeErrorToaster('Staging permission required',
    'Please configure staging permission in role');
};

