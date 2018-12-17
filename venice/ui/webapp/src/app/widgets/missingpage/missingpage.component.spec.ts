import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { MissingpageComponent } from './missingpage.component';
import { ControllerService } from '@app/services/controller.service';
import { SharedModule } from '@app/components/shared/shared.module';
import { MatIconRegistry } from '@angular/material';
import { LogPublishersService } from '@app/services/logging/log-publishers.service';
import { LogService } from '@app/services/logging/log.service';
import { RouterTestingModule } from '@angular/router/testing';
import { MessageService } from 'primeng/primeng';

describe('MissingpageComponent', () => {
  let component: MissingpageComponent;
  let fixture: ComponentFixture<MissingpageComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [MissingpageComponent],
      imports: [
        SharedModule,
        RouterTestingModule,
      ],
      providers: [
        ControllerService,
        LogService,
        LogPublishersService,
        MatIconRegistry,
        MessageService
      ]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(MissingpageComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
