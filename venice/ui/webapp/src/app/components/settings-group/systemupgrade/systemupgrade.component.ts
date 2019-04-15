import { Component, OnDestroy, OnInit , ViewEncapsulation} from '@angular/core';
import { Animations } from '@app/animations';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { ControllerService } from '@app/services/controller.service';

@Component({
  selector: 'app-systemupgrade',
  templateUrl: './systemupgrade.component.html',
  styleUrls: ['./systemupgrade.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class SystemupgradeComponent implements OnInit, OnDestroy {

  constructor(protected _controllerService: ControllerService ) {    }
    ngOnInit() {
    }

    /**
     * Component is about to exit
     */
    ngOnDestroy() {
      // publish event that AppComponent is about to exist
      this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, { 'component': 'SystemupgradeComponent', 'state': Eventtypes.COMPONENT_DESTROY });
    }
}
