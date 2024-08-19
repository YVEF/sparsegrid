import models
import os
import torch
import pandas as pd
import gzip
import io


WEIGHT_FILE = "sgw_native.nn"


def sg_convert_weights():
    model = models.SgModel()
    if not os.path.isfile(model.SG_MODEL_WEIGHTS_FILE):
        return 1
    if os.path.isfile(WEIGHT_FILE):
        os.remove(WEIGHT_FILE)

    model.load_state_dict(torch.load(model.SG_MODEL_WEIGHTS_FILE, weights_only=True))
    state_ = model.state_dict()
    data = []
    for t in state_:
        param_ = state_[t].numpy().flatten()
        shape_ = [*state_[t].shape] if len(state_[t].shape) == 2 else [*state_[t].shape, 1]
        assert len(shape_) == 2
        data.append([t, *shape_, *param_])

    df = pd.DataFrame(data)
    bb = bytes(df.to_csv(index=False, header=False), encoding='utf-8')
    res = gzip.compress(bb)
    with open(WEIGHT_FILE, "wb") as file:
        file.write(res)

    return 0
