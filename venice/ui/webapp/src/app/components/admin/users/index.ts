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

