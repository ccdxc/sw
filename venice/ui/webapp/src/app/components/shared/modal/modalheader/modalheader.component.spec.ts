import { async, ComponentFixture, TestBed } from '@angular/core/testing';
import { MaterialdesignModule } from '@lib/materialdesign.module';

import { ModalheaderComponent } from './modalheader.component';

describe('ModalheaderComponent', () => {
  let component: ModalheaderComponent;
  let fixture: ComponentFixture<ModalheaderComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ModalheaderComponent],
      imports: [MaterialdesignModule,]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(ModalheaderComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
