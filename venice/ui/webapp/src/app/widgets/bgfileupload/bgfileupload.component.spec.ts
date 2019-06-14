import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { BgfileuploadComponent } from './bgfileupload.component';
import { configureTestSuite } from 'ng-bullet';
import { SharedModule } from '@app/components/shared/shared.module';
import { PrimengModule } from '@app/lib/primeng.module';
import { ConfirmationService } from 'primeng/primeng';
import { MaterialdesignModule } from '@app/lib/materialdesign.module';
import { ControllerService } from '@app/services/controller.service';
import { RouterTestingModule } from '@angular/router/testing';
import { LogService } from '@app/services/logging/log.service';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { MessageService } from '@app/services/message.service';

describe('BgfileuploadComponent', () => {

  let component: BgfileuploadComponent;
  let fixture: ComponentFixture<BgfileuploadComponent>;

  configureTestSuite(() => {
    TestBed.configureTestingModule({
      declarations: [ BgfileuploadComponent
      ],
      imports: [
        SharedModule,
        PrimengModule,
        MaterialdesignModule,
        RouterTestingModule
      ],
      providers: [
        ControllerService,
        LogService,
        LogPublishersService,
        MessageService,
        ConfirmationService
      ],
    });

    });

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ BgfileuploadComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(BgfileuploadComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
